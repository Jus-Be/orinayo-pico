/*
 * M5Stack Audio Player (U197) API
 * Based on the N9301 serial protocol over UART1 (GPIO 4=TX, GPIO 5=RX).
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#pragma once

#include <stdbool.h>
#include <stdint.h>

/* UART1 pin assignments for the M5Stack Audio Player (U197) */
#define M5AUDIO_UART_ID      uart1
#define M5AUDIO_BAUD_RATE    9600
#define M5AUDIO_UART_TX_PIN  4
#define M5AUDIO_UART_RX_PIN  5

/* Volume range accepted by m5audio_set_volume() */
#define M5AUDIO_VOLUME_MIN  0
#define M5AUDIO_VOLUME_MAX  30

/* Equalizer presets for m5audio_set_eq() */
typedef enum {
    M5AUDIO_EQ_NORMAL  = 0,
    M5AUDIO_EQ_POP     = 1,
    M5AUDIO_EQ_ROCK    = 2,
    M5AUDIO_EQ_JAZZ    = 3,
    M5AUDIO_EQ_CLASSIC = 4,
    M5AUDIO_EQ_BASS    = 5,
} m5audio_eq_t;

/*
 * Initialise UART1 at 9600 baud on GPIO 4 (TX) and GPIO 5 (RX) and
 * prepare the N9301 audio module.  Must be called before any other
 * m5audio_* function.
 */
void m5audio_init(void);

/* Resume or start playback. */
void m5audio_play(void);

/* Pause playback. */
void m5audio_pause(void);

/* Stop playback. */
void m5audio_stop(void);

/* Advance to the next track. */
void m5audio_next(void);

/* Return to the previous track. */
void m5audio_prev(void);

/*
 * Play a specific track by its 1-based index (1–3000).
 * The index corresponds to the order in which files are stored on the
 * SD card / flash of the audio module.
 * Values outside [1, 3000] are silently ignored.
 */
void m5audio_play_track(uint16_t track);

/*
 * Play a file inside a numbered folder.
 *   folder : folder number (1–99)
 *   file   : file number   (1–255)
 * Values outside these ranges are silently ignored.
 */
void m5audio_play_folder(uint8_t folder, uint8_t file);

/*
 * Set the output volume.
 * volume must be in the range [M5AUDIO_VOLUME_MIN, M5AUDIO_VOLUME_MAX].
 */
void m5audio_set_volume(uint8_t volume);

/* Increase volume by one step. */
void m5audio_volume_up(void);

/* Decrease volume by one step. */
void m5audio_volume_down(void);

/*
 * Select an equalizer preset.
 * eq must be one of the m5audio_eq_t values.
 */
void m5audio_set_eq(m5audio_eq_t eq);

/* Reset the audio module. */
void m5audio_reset(void);
