/*
 * M5Stack Audio Player (U197) – N9301 protocol driver
 *
 * The N9301 uses a fixed-length 10-byte serial frame:
 *
 *   Byte  0 : 0x7E  (start)
 *   Byte  1 : 0xFF  (version)
 *   Byte  2 : 0x06  (data length, always 6)
 *   Byte  3 : CMD   (command)
 *   Byte  4 : 0x00  (feedback disabled)
 *   Byte  5 : para1 (parameter high byte)
 *   Byte  6 : para2 (parameter low byte)
 *   Bytes 7–8 : two's-complement checksum of bytes 1–6
 *   Byte  9 : 0xEF  (end)
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "m5audio.h"

#include "hardware/gpio.h"
#include "hardware/uart.h"
#include "pico/stdlib.h"

/* ---- N9301 protocol constants ------------------------------------------- */

#define N9301_START    0x7E
#define N9301_VERSION  0xFF
#define N9301_LENGTH   0x06
#define N9301_FEEDBACK 0x00
#define N9301_END      0xEF

/* N9301 command codes */
#define N9301_CMD_PLAY_NEXT    0x01
#define N9301_CMD_PLAY_PREV    0x02
#define N9301_CMD_PLAY_TRACK   0x03
#define N9301_CMD_VOL_UP       0x04
#define N9301_CMD_VOL_DOWN     0x05
#define N9301_CMD_SET_VOLUME   0x06
#define N9301_CMD_SET_EQ       0x07
#define N9301_CMD_PLAY_FOLDER  0x0F
#define N9301_CMD_STOP         0x16
#define N9301_CMD_RESET        0x0C
#define N9301_CMD_PLAY         0x0D
#define N9301_CMD_PAUSE        0x0E

/* ---- Internal helpers ---------------------------------------------------- */

/*
 * Build and transmit a 10-byte N9301 frame.
 *
 *   cmd   : command byte
 *   para1 : high byte of the 16-bit parameter
 *   para2 : low  byte of the 16-bit parameter
 */
static void n9301_send(uint8_t cmd, uint8_t para1, uint8_t para2) {
    uint8_t frame[10];

    frame[0] = N9301_START;
    frame[1] = N9301_VERSION;
    frame[2] = N9301_LENGTH;
    frame[3] = cmd;
    frame[4] = N9301_FEEDBACK;
    frame[5] = para1;
    frame[6] = para2;

    /* Two's-complement checksum of bytes 1–6 */
    uint16_t sum = 0;
    for (int i = 1; i <= 6; i++) {
        sum += frame[i];
    }
    uint16_t checksum = (~sum) + 1u;
    frame[7] = (uint8_t)((checksum >> 8) & 0xFF);
    frame[8] = (uint8_t)(checksum & 0xFF);

    frame[9] = N9301_END;

    for (int i = 0; i < 10; i++) {
        while (!uart_is_writable(M5AUDIO_UART_ID)) { /* wait */ }
        uart_putc(M5AUDIO_UART_ID, frame[i]);
    }
}

/* ---- Public API ---------------------------------------------------------- */

void m5audio_init(void) {
    uart_init(M5AUDIO_UART_ID, M5AUDIO_BAUD_RATE);
    gpio_set_function(M5AUDIO_UART_TX_PIN, GPIO_FUNC_UART);
    gpio_set_function(M5AUDIO_UART_RX_PIN, GPIO_FUNC_UART);
    uart_set_fifo_enabled(M5AUDIO_UART_ID, true);
    uart_set_translate_crlf(M5AUDIO_UART_ID, false);
    /* Allow the module time to finish its power-on initialisation */
    sleep_ms(500);
}

void m5audio_play(void) {
    n9301_send(N9301_CMD_PLAY, 0x00, 0x00);
}

void m5audio_pause(void) {
    n9301_send(N9301_CMD_PAUSE, 0x00, 0x00);
}

void m5audio_stop(void) {
    n9301_send(N9301_CMD_STOP, 0x00, 0x00);
}

void m5audio_next(void) {
    n9301_send(N9301_CMD_PLAY_NEXT, 0x00, 0x00);
}

void m5audio_prev(void) {
    n9301_send(N9301_CMD_PLAY_PREV, 0x00, 0x00);
}

void m5audio_play_track(uint16_t track) {
    if (track < 1 || track > 3000) {
        return;
    }
    n9301_send(N9301_CMD_PLAY_TRACK,
               (uint8_t)((track >> 8) & 0xFF),
               (uint8_t)(track & 0xFF));
}

void m5audio_play_folder(uint8_t folder, uint8_t file) {
    if (folder < 1 || folder > 99 || file < 1) {
        return;
    }
    n9301_send(N9301_CMD_PLAY_FOLDER, folder, file);
}

void m5audio_set_volume(uint8_t volume) {
    if (volume > M5AUDIO_VOLUME_MAX) {
        volume = M5AUDIO_VOLUME_MAX;
    }
    n9301_send(N9301_CMD_SET_VOLUME, 0x00, volume);
}

void m5audio_volume_up(void) {
    n9301_send(N9301_CMD_VOL_UP, 0x00, 0x00);
}

void m5audio_volume_down(void) {
    n9301_send(N9301_CMD_VOL_DOWN, 0x00, 0x00);
}

void m5audio_set_eq(m5audio_eq_t eq) {
    n9301_send(N9301_CMD_SET_EQ, 0x00, (uint8_t)eq);
}

void m5audio_reset(void) {
    n9301_send(N9301_CMD_RESET, 0x00, 0x00);
    /* Allow the module time to complete its reset */
    sleep_ms(500);
}
