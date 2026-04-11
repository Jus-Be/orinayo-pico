/*
 * Copyright 2025, Hiroyuki OYAMA
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Persistent flash storage engine for BTstack Classic BT link keys.
 *
 * Flash layout (relative to end of flash):
 *   Sector -9  (this file)  BT link keys  – BT_KEYS_FLASH_OFFSET
 *   Sector -8 .. -1         Game storage  – GHOST_FLASH_BANK_STORAGE_OFFSET
 *
 * Each btstack_link_key_db_t operation that modifies data follows the
 * pattern used in storage.c:
 *   1. Copy the current sector to a RAM buffer.
 *   2. Apply the change in RAM.
 *   3. Erase the sector and reprogram it via flash_safe_execute so the
 *      CYW43 SPI bus is safely paused for the duration.
 */
#include "bt_link_key_store.h"

#include <string.h>

#include "hardware/flash.h"
#include "pico/flash.h"

/* One flash sector (4 KB) immediately before the game storage sector. */
#define BT_KEYS_FLASH_OFFSET  (PICO_FLASH_SIZE_BYTES - (FLASH_SECTOR_SIZE * 9u))

#define BT_KEYS_MAGIC         0x424B4559u   /* "BKEY" */
#define BT_NVM_NUM_LINK_KEYS  16

/*
 * Storage occupies exactly 2 flash pages (512 bytes) so that
 * flash_range_program (which requires a multiple of FLASH_PAGE_SIZE) works
 * cleanly.  The struct is padded to fill the full 512 bytes.
 */
#define BT_KEYS_STORAGE_PAGES 2u
#define BT_KEYS_STORAGE_SIZE  (BT_KEYS_STORAGE_PAGES * FLASH_PAGE_SIZE)  /* 512 */

typedef struct {
    uint8_t valid;                  /* 1 = slot occupied                     */
    uint8_t bd_addr[BD_ADDR_LEN];  /* Bluetooth device address (6 bytes)    */
    uint8_t link_key[LINK_KEY_LEN];/* Link key (16 bytes)                   */
    uint8_t type;                  /* link_key_type_t, stored as raw byte   */
} __attribute__((packed)) bt_key_entry_t;  /* 24 bytes */

#define BT_KEY_STORE_DATA_SIZE  (sizeof(uint32_t) + BT_NVM_NUM_LINK_KEYS * sizeof(bt_key_entry_t))
#define BT_KEY_STORE_PAD        (BT_KEYS_STORAGE_SIZE - BT_KEY_STORE_DATA_SIZE)

typedef struct {
    uint32_t       magic;
    bt_key_entry_t entries[BT_NVM_NUM_LINK_KEYS];
    uint8_t        _pad[BT_KEY_STORE_PAD];
} __attribute__((packed)) bt_key_store_t;

_Static_assert(sizeof(bt_key_store_t) == BT_KEYS_STORAGE_SIZE,
               "bt_key_store_t must be exactly BT_KEYS_STORAGE_SIZE bytes");

/* -------------------------------------------------------------------------
 * Flash helpers – must run from RAM (not XIP) while flash is being written.
 * -------------------------------------------------------------------------*/

typedef struct {
    bool      erase_only;
    uintptr_t offset;      /* flash offset (not XIP address)  */
    uintptr_t data_ptr;    /* source pointer in SRAM          */
    uint32_t  data_size;   /* bytes to program (pages-aligned) */
} bt_flash_op_t;

static void __no_inline_not_in_flash_func(bt_perform_flash_op)(void *param) {
    const bt_flash_op_t *op = (const bt_flash_op_t *)param;
    flash_range_erase(op->offset, FLASH_SECTOR_SIZE);
    if (!op->erase_only) {
        flash_range_program(op->offset, (const uint8_t *)op->data_ptr, op->data_size);
    }
}

/* RAM scratch-pad used for read-modify-write cycles.  512 bytes in SRAM. */
static uint8_t bt_key_storage_buf[BT_KEYS_STORAGE_SIZE];

static const bt_key_store_t *flash_store(void) {
    return (const bt_key_store_t *)(XIP_BASE + BT_KEYS_FLASH_OFFSET);
}

/* Copy flash data to RAM (or initialise a fresh store if magic is absent). */
static void bt_load_to_ram(void) {
    const bt_key_store_t *fs = flash_store();
    if (fs->magic == BT_KEYS_MAGIC) {
        memcpy(bt_key_storage_buf, fs, BT_KEYS_STORAGE_SIZE);
    } else {
        memset(bt_key_storage_buf, 0, BT_KEYS_STORAGE_SIZE);
        ((bt_key_store_t *)bt_key_storage_buf)->magic = BT_KEYS_MAGIC;
    }
}

/* Erase sector and reprogram from the RAM buffer. */
static void bt_flush_to_flash(void) {
    bt_flash_op_t op = {
        .erase_only = false,
        .offset     = BT_KEYS_FLASH_OFFSET,
        .data_ptr   = (uintptr_t)bt_key_storage_buf,
        .data_size  = BT_KEYS_STORAGE_SIZE,
    };
    flash_safe_execute(bt_perform_flash_op, &op, UINT32_MAX);
}

/* -------------------------------------------------------------------------
 * btstack_link_key_db_t implementation
 * -------------------------------------------------------------------------*/

static void bt_lk_open(void) {}
static void bt_lk_close(void) {}
static void bt_lk_set_local_bd_addr(bd_addr_t bd_addr) { (void)bd_addr; }

static int bt_lk_get_link_key(bd_addr_t bd_addr, link_key_t link_key, link_key_type_t *type) {
    const bt_key_store_t *store = flash_store();
    if (store->magic != BT_KEYS_MAGIC) return 0;
    for (int i = 0; i < BT_NVM_NUM_LINK_KEYS; i++) {
        if (store->entries[i].valid &&
            memcmp(store->entries[i].bd_addr, bd_addr, BD_ADDR_LEN) == 0) {
            memcpy(link_key, store->entries[i].link_key, LINK_KEY_LEN);
            *type = (link_key_type_t)store->entries[i].type;
            return 1;
        }
    }
    return 0;
}

static void bt_lk_put_link_key(bd_addr_t bd_addr, link_key_t link_key, link_key_type_t type) {
    bt_load_to_ram();
    bt_key_store_t *store = (bt_key_store_t *)bt_key_storage_buf;

    /* Find existing entry for this address or the first free slot. */
    int slot = -1;
    for (int i = 0; i < BT_NVM_NUM_LINK_KEYS; i++) {
        if (store->entries[i].valid &&
            memcmp(store->entries[i].bd_addr, bd_addr, BD_ADDR_LEN) == 0) {
            slot = i;
            break;
        }
        if (!store->entries[i].valid && slot < 0) {
            slot = i;
        }
    }
    /* If the store is full, evict slot 0 (simple eviction policy). */
    if (slot < 0) slot = 0;

    store->entries[slot].valid = 1;
    memcpy(store->entries[slot].bd_addr,  bd_addr,  BD_ADDR_LEN);
    memcpy(store->entries[slot].link_key, link_key, LINK_KEY_LEN);
    store->entries[slot].type = (uint8_t)type;

    bt_flush_to_flash();
}

static void bt_lk_delete_link_key(bd_addr_t bd_addr) {
    if (flash_store()->magic != BT_KEYS_MAGIC) return;
    bt_load_to_ram();
    bt_key_store_t *store = (bt_key_store_t *)bt_key_storage_buf;
    bool changed = false;
    for (int i = 0; i < BT_NVM_NUM_LINK_KEYS; i++) {
        if (store->entries[i].valid &&
            memcmp(store->entries[i].bd_addr, bd_addr, BD_ADDR_LEN) == 0) {
            store->entries[i].valid = 0;
            changed = true;
        }
    }
    if (changed) bt_flush_to_flash();
}

static int bt_lk_iterator_init(btstack_link_key_iterator_t *it) {
    it->context = (void *)0;
    return flash_store()->magic == BT_KEYS_MAGIC ? 1 : 0;
}

static int bt_lk_iterator_get_next(btstack_link_key_iterator_t *it,
                                   bd_addr_t bd_addr, link_key_t link_key,
                                   link_key_type_t *type) {
    const bt_key_store_t *store = flash_store();
    uintptr_t idx = (uintptr_t)it->context;
    while (idx < BT_NVM_NUM_LINK_KEYS) {
        if (store->entries[idx].valid) {
            memcpy(bd_addr,  store->entries[idx].bd_addr,  BD_ADDR_LEN);
            memcpy(link_key, store->entries[idx].link_key, LINK_KEY_LEN);
            *type = (link_key_type_t)store->entries[idx].type;
            it->context = (void *)(idx + 1);
            return 1;
        }
        idx++;
    }
    return 0;
}

static void bt_lk_iterator_done(btstack_link_key_iterator_t *it) { (void)it; }

static void bt_lk_delete_all(void) {
    bt_flash_op_t op = {
        .erase_only = true,
        .offset     = BT_KEYS_FLASH_OFFSET,
        .data_ptr   = 0,
        .data_size  = 0,
    };
    flash_safe_execute(bt_perform_flash_op, &op, UINT32_MAX);
}

/* -------------------------------------------------------------------------
 * Public interface
 * -------------------------------------------------------------------------*/

static const btstack_link_key_db_t bt_link_key_db = {
    .open              = bt_lk_open,
    .close             = bt_lk_close,
    .set_local_bd_addr = bt_lk_set_local_bd_addr,
    .get_link_key      = bt_lk_get_link_key,
    .put_link_key      = bt_lk_put_link_key,
    .delete_link_key   = bt_lk_delete_link_key,
    .iterator_init     = bt_lk_iterator_init,
    .iterator_get_next = bt_lk_iterator_get_next,
    .iterator_done     = bt_lk_iterator_done,
};

const btstack_link_key_db_t *bt_link_key_store_get_db(void) {
    return &bt_link_key_db;
}
