/*
 * Copyright 2025, Hiroyuki OYAMA
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#include <string.h>

#include "hardware/flash.h"
#include "looper.h"
#include "pico/flash.h"
#include <pico/cyw43_arch.h>

#ifndef GHOST_FLASH_BANK_STORAGE_OFFSET
#define GHOST_FLASH_BANK_STORAGE_OFFSET (PICO_FLASH_SIZE_BYTES - (FLASH_SECTOR_SIZE * 8))
#endif

#define MAGIC_HEADER "GHST"
#define NUM_TRACKS 4

typedef struct {
    uint32_t magic;	
    bool preferences[FLASH_PAGE_SIZE - 4];
} storage_preference_t;

typedef struct {
    uint32_t magic;
    bool pattern[NUM_TRACKS][LOOPER_TOTAL_STEPS];
} storage_pattern_t;

typedef struct {
    bool op_is_erase;
    uintptr_t p0;
    uintptr_t p1;
} mutation_operation_t;

extern bool enable_ample_guitar;
extern bool enable_midi_drums;
extern bool enable_seqtrak;
extern bool enable_chord_track;
extern bool enable_bass_track;
extern bool enable_modx;
extern bool enable_sp404mk2;

void midi_send_note(uint8_t command, uint8_t note, uint8_t velocity);

static void __no_inline_not_in_flash_func(flash_bank_perform_operation)(void *param) {
    const mutation_operation_t *mop = (const mutation_operation_t *)param;
	
    if (mop->op_is_erase) {
        flash_range_erase(mop->p0, FLASH_SECTOR_SIZE);
    } else {
        flash_range_erase(mop->p0, FLASH_SECTOR_SIZE);		
        flash_range_program(mop->p0, (const uint8_t *)mop->p1, FLASH_PAGE_SIZE);
    }
	
	midi_send_note(0x96, 66, 66);	
}


bool storage_erase_tracks(void) {
    mutation_operation_t erase = {.op_is_erase = true, .p0 = GHOST_FLASH_BANK_STORAGE_OFFSET};
    flash_safe_execute(flash_bank_perform_operation, &erase, UINT32_MAX);
	//flash_bank_perform_operation(&erase);
	midi_send_note(0x92, 33, 33);	
    return true;
}

bool storage_store_preferences(void) {
    uint8_t storage[FLASH_PAGE_SIZE];
    storage_preference_t *data = (storage_preference_t *)&storage;	

    memcpy(&data->magic, MAGIC_HEADER, sizeof(data->magic));
	
	data->preferences[0] = enable_ample_guitar;
	data->preferences[1] = enable_midi_drums;
	data->preferences[2] = enable_seqtrak;
	data->preferences[3] = enable_modx;
	data->preferences[4] = enable_sp404mk2;
	
    mutation_operation_t program = {.op_is_erase = false, .p0 = GHOST_FLASH_BANK_STORAGE_OFFSET, .p1 = (uintptr_t)storage};
	
    //int32_t result = flash_safe_execute(flash_bank_perform_operation, &program, UINT32_MAX);
	//midi_send_note(0x95, enable_ample_guitar ? 127 : 0, result == PICO_OK ? 91 : (result == PICO_ERROR_NOT_PERMITTED ? 92 : (result == PICO_ERROR_TIMEOUT ? 93 : 94)));

	cyw43_arch_deinit();						// SHUTDOWN Bluetooth
	
	flash_bank_perform_operation(&program);	
	midi_send_note(0x95, enable_ample_guitar ? 127 : 0, enable_ample_guitar ? 127 : 0);
	cyw43_arch_init();		
    return true;
}

bool storage_load_preferences(void) {
    const storage_preference_t *data = (const storage_preference_t *)(XIP_BASE + GHOST_FLASH_BANK_STORAGE_OFFSET);
	midi_send_note(0x93, 44, 44);
	
    if (memcmp(&data->magic, MAGIC_HEADER, sizeof(data->magic)) != 0) {
        return false;
	}
	
	enable_ample_guitar = data->preferences[0];
	enable_midi_drums 	= data->preferences[1];
	enable_seqtrak 		= data->preferences[2];
	enable_modx 		= data->preferences[3];
	enable_sp404mk2 	= data->preferences[4];
	
	midi_send_note(0x94, data->preferences[0] ? 127 : 0, enable_ample_guitar ? 127 : 0);	
    return true;
}

bool storage_load_tracks(void) {
    size_t num_tracks;
    track_t *tracks = looper_tracks_get(&num_tracks);

    const storage_pattern_t *data = (const storage_pattern_t *)(XIP_BASE + GHOST_FLASH_BANK_STORAGE_OFFSET);
	
    if (memcmp(&data->magic, MAGIC_HEADER, sizeof(data->magic)) != 0)
        return false;

    for (size_t t = 0; t < num_tracks; t++) 
	{
        for (size_t i = 0; i < LOOPER_TOTAL_STEPS; i++) 
		{
            tracks[t].pattern[i] = data->pattern[t][i];
        }
    }
    return true;
}

bool storage_store_tracks(void) {
    uint8_t storage[FLASH_PAGE_SIZE];
    storage_pattern_t *data = (storage_pattern_t *)&storage;
    size_t num_tracks;
    track_t *tracks = looper_tracks_get(&num_tracks);

    memcpy(&data->magic, MAGIC_HEADER, sizeof(data->magic));
	
    for (size_t t = 0; t < NUM_TRACKS; t++) {
        memcpy(data->pattern[t], tracks[t].pattern, sizeof(tracks[t].pattern));
    }
	
    mutation_operation_t program = {.op_is_erase = false, .p0 = GHOST_FLASH_BANK_STORAGE_OFFSET, .p1 = (uintptr_t)storage};
    flash_safe_execute(flash_bank_perform_operation, &program, UINT32_MAX);

    return true;
}
