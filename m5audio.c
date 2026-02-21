/*
 * M5Stack Audio Player (U197) driver
 * Compatible with https://github.com/m5stack/M5Unit-AudioPlayer
 *
 * Protocol frame format (variable length):
 *
 *   Byte  0     : CMD              (command)
 *   Byte  1     : ~CMD & 0xFF      (one's complement of command)
 *   Byte  2     : LEN              (number of data bytes that follow)
 *   Bytes 3..N  : data bytes       (LEN bytes)
 *   Byte  N+1   : checksum         (sum of all preceding bytes, & 0xFF)
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "m5audio.h"

#include "hardware/gpio.h"
#include "hardware/uart.h"
#include "pico/stdlib.h"

/* Maximum number of data bytes per frame and total frame buffer size */
#define AUDIOPLAYER_MAX_DATA    32
#define AUDIOPLAYER_FRAME_SIZE  (4 + AUDIOPLAYER_MAX_DATA)  /* cmd + ~cmd + len + data + checksum */

/* ---- Internal helpers ---------------------------------------------------- */

/*
 * Build and transmit a variable-length frame.
 *
 *   cmd     : command byte
 *   data    : pointer to the data bytes
 *   datalen : number of data bytes (capped at AUDIOPLAYER_MAX_DATA)
 */
static void audioplayer_send(uint8_t cmd, const uint8_t *data, size_t datalen) {
    uint8_t frame[AUDIOPLAYER_FRAME_SIZE];
    if (datalen > AUDIOPLAYER_MAX_DATA) {
        datalen = AUDIOPLAYER_MAX_DATA;
    }

    frame[0] = cmd;
    frame[1] = (~cmd) & 0xFF;
    frame[2] = (uint8_t)datalen;

    uint8_t sum = frame[0] + frame[1] + frame[2];
    for (size_t i = 0; i < datalen; i++) {
        frame[3 + i] = data[i];
        sum += data[i];
    }
    frame[3 + datalen] = sum & 0xFF;

    for (size_t i = 0; i < 4 + datalen; i++) {
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
    uint8_t data[] = {0x01};
    audioplayer_send(0x04, data, 1);
}

void m5audio_pause(void) {
    uint8_t data[] = {0x02};
    audioplayer_send(0x04, data, 1);
}

void m5audio_stop(void) {
    uint8_t data[] = {0x03};
    audioplayer_send(0x04, data, 1);
}

void m5audio_next(void) {
    uint8_t data[] = {0x05};
    audioplayer_send(0x04, data, 1);
}

void m5audio_prev(void) {
    uint8_t data[] = {0x04};
    audioplayer_send(0x04, data, 1);
}

void m5audio_play_track(uint16_t track) {
    if (track < 1 || track > 3000) {
        return;
    }
    uint8_t data[] = {0x06,
                      (uint8_t)((track >> 8) & 0xFF),
                      (uint8_t)(track & 0xFF)};
    audioplayer_send(0x04, data, 3);
}

void m5audio_set_volume(uint8_t volume) {
    if (volume > M5AUDIO_VOLUME_MAX) {
        volume = M5AUDIO_VOLUME_MAX;
    }
    uint8_t data[] = {0x01, volume};
    audioplayer_send(0x06, data, 2);
}

void m5audio_volume_up(void) {
    uint8_t data[] = {0x02};
    audioplayer_send(0x06, data, 1);
}

void m5audio_volume_down(void) {
    uint8_t data[] = {0x03};
    audioplayer_send(0x06, data, 1);
}

void m5audio_set_play_mode(m5audio_play_mode_t mode) {
    uint8_t data[] = {0x01, (uint8_t)mode};
    audioplayer_send(0x0B, data, 2);
}
