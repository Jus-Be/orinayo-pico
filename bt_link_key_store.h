/*
 * Copyright 2025, Hiroyuki OYAMA
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Persistent flash storage for BTstack Classic BT link keys.
 * Implements btstack_link_key_db_t so that paired devices (e.g. a guitar
 * controller) reconnect automatically after a power cycle.
 *
 * Register with BTstack before hci_power_control:
 *   gap_set_link_key_db(bt_link_key_store_get_db());
 */
#pragma once

#include <btstack.h>

const btstack_link_key_db_t *bt_link_key_store_get_db(void);
