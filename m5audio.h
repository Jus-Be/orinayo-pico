/*
 * M5Stack Audio Player (U197) API
 * Compatible with https://github.com/m5stack/M5Unit-AudioPlayer
 * Communicates over UART1 (GPIO 4=TX, GPIO 5=RX) at 9600 baud.
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

/* Playback loop mode presets for m5audio_set_play_mode() */
typedef enum {
    M5AUDIO_PLAY_MODE_ALL_LOOP    = 0,  /* Play all tracks in order, loop after finishing */
    M5AUDIO_PLAY_MODE_SINGLE_LOOP = 1,  /* Loop the current track */
    M5AUDIO_PLAY_MODE_FOLDER_LOOP = 2,  /* Loop all tracks in the current folder */
    M5AUDIO_PLAY_MODE_RANDOM      = 3,  /* Play tracks randomly from the entire disk */
    M5AUDIO_PLAY_MODE_SINGLE_STOP = 4,  /* Play current track once and stop */
    M5AUDIO_PLAY_MODE_ALL_ONCE    = 5,  /* Play all tracks in order once, then stop */
    M5AUDIO_PLAY_MODE_FOLDER_ONCE = 6,  /* Play all tracks in the current folder once, then stop */
} m5audio_play_mode_t;

/*
 * Initialise UART1 at 9600 baud on GPIO 4 (TX) and GPIO 5 (RX) and
 * prepare the audio module.  Must be called before any other
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
 * SD card of the audio module.
 * Values outside [1, 3000] are silently ignored.
 */
void m5audio_play_track(uint16_t track);

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
 * Set the playback loop mode.
 * mode must be one of the m5audio_play_mode_t values.
 */
void m5audio_set_play_mode(m5audio_play_mode_t mode);
