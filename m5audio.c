/*
 * Grove MP3 Module V4.0 (WT2605C) driver
 * Compatible with https://wiki.seeedstudio.com/Grove-MP3-v4.0/
 *
 * Protocol: ASCII AT commands sent over UART, terminated with CR+LF.
 * Command format:  AT+<CMD>[=<params>]\r\n
 * Response:        "OK\r\n" on success, "ERR\r\n" on failure.
 *
 * Responses are not read back; the RX line is used by the module only.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "m5audio.h"

#include <stdio.h>
#include <string.h>

#include "hardware/gpio.h"
#include "hardware/uart.h"
#include "pico/stdlib.h"

/* Maximum length of a single AT command string (excluding \r\n) */
#define WT2605C_CMD_MAX  64

/*
 * WT2605C REPEATMODE values (1-based) for each m5audio_play_mode_t entry.
 *
 * WT2605C modes:
 *   1 = all-loop (cycle)
 *   2 = single-loop
 *   3 = folder-loop (dir cycle)
 *   4 = random
 *   5 = single-shot (play once, stop)
 *
 * M5AUDIO_PLAY_MODE_ALL_ONCE and M5AUDIO_PLAY_MODE_FOLDER_ONCE have no
 * exact equivalents; they are mapped to the closest WT2605C modes.
 */
static const uint8_t play_mode_map[] = {
    1,  /* ALL_LOOP    -> cycle all          */
    2,  /* SINGLE_LOOP -> single cycle       */
    3,  /* FOLDER_LOOP -> dir cycle          */
    4,  /* RANDOM      -> random             */
    5,  /* SINGLE_STOP -> single shot        */
    5,  /* ALL_ONCE    -> single shot (approx) */
    3,  /* FOLDER_ONCE -> dir cycle (approx) */
};

/* Ensure play_mode_map stays in sync with m5audio_play_mode_t */
static_assert(sizeof(play_mode_map) == M5AUDIO_PLAY_MODE_FOLDER_ONCE + 1,
              "play_mode_map must have one entry per m5audio_play_mode_t value");

/* ---- Internal helpers ---------------------------------------------------- */

/*
 * Transmit a complete AT command string followed by CR+LF.
 * cmd must be a null-terminated string containing everything after "AT+".
 * Example: wt2605c_send("STOP") -> sends "AT+STOP\r"
 */
static void wt2605c_send(const char *cmd) {
    uart_write_blocking(M5AUDIO_UART_ID, (const uint8_t *)cmd, strlen(cmd));
    uart_tx_wait_blocking(M5AUDIO_UART_ID); 	
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
    wt2605c_send("AT+PP\r");    
}

void m5audio_pause(void) {
    wt2605c_send("AT+PP\r");    
}

void m5audio_stop(void) {
    wt2605c_send("AT+STOP\r");
    
}

void m5audio_next(void) {
    wt2605c_send("AT+NEXT\r");
    
}

void m5audio_prev(void) {
    wt2605c_send("AT+PREV\r");
    
}

void m5audio_loop_track(uint16_t track) {
    if (track < 1 || track > 3000) {
        return;
    }
    char buf[WT2605C_CMD_MAX];
    snprintf(buf, sizeof(buf), "AT+LPLAY=sd0,%u\r", (unsigned int)track);
    wt2605c_send(buf);
}

void m5audio_play_track(uint16_t track) {
    if (track < 1 || track > 3000) {
        return;
    }
    char buf[WT2605C_CMD_MAX];
    snprintf(buf, sizeof(buf), "AT+PLAY=sd0,%u\r", (unsigned int)track);
    wt2605c_send(buf);
}

void m5audio_set_volume(uint8_t volume) {
    if (volume > M5AUDIO_VOLUME_MAX) {
        volume = M5AUDIO_VOLUME_MAX;
    }
    char buf[WT2605C_CMD_MAX];
    snprintf(buf, sizeof(buf), "AT+VOL=%u\r", (unsigned int)volume);
    wt2605c_send(buf);  
}

void m5audio_volume_up(void) {
    wt2605c_send("AT+VOLUP\r");    
}

void m5audio_volume_down(void) {
    wt2605c_send("AT+VOLDOWN\r");
    
}

void m5audio_set_play_mode(m5audio_play_mode_t mode) {
    uint8_t wt_mode;
    if ((unsigned int)mode < sizeof(play_mode_map)) {
        wt_mode = play_mode_map[mode];
    } else {
        wt_mode = 1;
    }
    char buf[WT2605C_CMD_MAX];
    snprintf(buf, sizeof(buf), "AT+REPEATMODE=%u\r", (unsigned int)wt_mode);
    wt2605c_send(buf);
    
}

void m5audio_play_audio_by_name(uint8_t *name, uint8_t name_len) {
    char buf[WT2605C_CMD_MAX];
    int prefix_len = snprintf(buf, sizeof(buf), "AT+PLAY=sd0,");
    if (prefix_len < 0 || prefix_len >= (int)sizeof(buf)) {
        return;
    }
    size_t avail = sizeof(buf) - (size_t)prefix_len - 1; /* reserve space for null terminator */
    size_t copy_len = (name_len < avail) ? name_len : avail;
    memcpy(buf + prefix_len, name, copy_len);
    buf[prefix_len + copy_len] = '\r';
    buf[prefix_len + copy_len + 1] = '\0';	
    wt2605c_send(buf);
    
}
