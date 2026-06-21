/**
 * MIT License
 *
 * Copyright (c) 2023 rppicomidi
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#pragma once
#include <stdint.h>
#include "btstack_config.h"
#include "bluetooth.h"

#if defined __cplusplus
extern "C" {
#else
#include <stdbool.h>
#endif

#ifdef ENABLE_LE_DATA_LENGTH_EXTENSION
#define MAX_BLE_MIDI_PACKET 251-3
#else
#define MAX_BLE_MIDI_PACKET ATT_DEFAULT_MTU-3
#endif

typedef struct ble_midi_packet_s {
    uint16_t nbytes;
    uint8_t pkt[MAX_BLE_MIDI_PACKET];
} __attribute__((packed)) ble_midi_packet_t;

typedef struct ble_midi_codec_data_s ble_midi_codec_data_t;

typedef struct ble_midi_message_s {
    uint8_t nbytes;
    uint8_t msg_bytes[3];
    uint16_t timestamp_ms;
} __attribute__((packed)) ble_midi_message_t;

static const uint8_t ble_midi_packet_is_start_sysex = 0x80;
static const uint8_t ble_midi_packet_is_sysex        = 0x20;
static const uint8_t ble_midi_packet_is_real_time     = 0x10;
static const uint8_t ble_midi_packet_is_channel       = 0x08;
static const uint8_t ble_midi_packet_nbytes_mask      = 0x03;

ble_midi_codec_data_t* ble_midi_pkt_codec_get_data_by_index(uint8_t idx);
void     ble_midi_pkt_codec_init_data(ble_midi_codec_data_t* context, uint16_t ble_mtu);
void     ble_midi_pkt_codec_update_mtu(ble_midi_codec_data_t* context, uint16_t ble_mtu);
void     ble_midi_pkt_codec_set_mtu(ble_midi_codec_data_t* context, uint16_t ble_mtu);
uint16_t ble_midi_pkt_codec_get_mtu(ble_midi_codec_data_t* context);
uint16_t ble_midi_pkt_codec_push_midi(const uint8_t* midi_stream, uint16_t nbytes, ble_midi_codec_data_t* context, bool* ready_to_send);
uint16_t ble_midi_pkt_codec_pop_midi(ble_midi_message_t* mes, ble_midi_codec_data_t* context);
uint16_t ble_midi_pkt_codec_ble_midi_decode_push(const uint8_t* pkt, uint16_t nbytes, ble_midi_codec_data_t* context);
uint16_t ble_midi_pkt_codec_ble_pkt_pop(ble_midi_packet_t* pkt, ble_midi_codec_data_t* context);
bool     ble_midi_pkt_codec_ble_pkt_available(ble_midi_codec_data_t* context);

#if defined __cplusplus
}
#endif
