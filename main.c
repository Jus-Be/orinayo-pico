/**
 * Copyright (c) 2020 Raspberry Pi (Trading) Ltd.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stdio.h>
#include <string.h>
#include "pico/cyw43_arch.h"
#include "pico/multicore.h"
#include "hardware/clocks.h"
#include "hardware/i2c.h"
#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include "pico/binary_info.h"
#include "pico/flash.h"
#include "bsp/board.h"
#include "tusb.h"
#include "pio_usb.h"
#include "pico_bluetooth.h"
#include "async_timer.h"
#include "storage.h"
#include "looper.h"
#include "note_scheduler.h"
#include "pico/stdlib.h"
#include "hardware/watchdog.h"
#include "hardware/i2c.h"

// Pico W devices use a GPIO on the WIFI chip for the LED,
// so when building for Pico W, CYW43_WL_GPIO_LED_PIN will be defined
#ifdef CYW43_WL_GPIO_LED_PIN
#include "pico/cyw43_arch.h"
#endif

#ifndef LED_DELAY_MS
#define LED_DELAY_MS 250
#endif

// Perform initialisation
int pico_led_init(void) {
#if defined(PICO_DEFAULT_LED_PIN)
    // A device like Pico that uses a GPIO for the LED will define PICO_DEFAULT_LED_PIN
    // so we can use normal GPIO functionality to turn the led on and off
    gpio_init(PICO_DEFAULT_LED_PIN);
    gpio_set_dir(PICO_DEFAULT_LED_PIN, GPIO_OUT);
    return PICO_OK;
#elif defined(CYW43_WL_GPIO_LED_PIN)
    // For Pico W devices we need to initialise the driver etc
    return cyw43_arch_init();
#endif
}

// Turn the led on or off
void pico_set_led(bool led_on) {
#if defined(PICO_DEFAULT_LED_PIN)
    // Just set the GPIO on or off
    gpio_put(PICO_DEFAULT_LED_PIN, led_on);
#elif defined(CYW43_WL_GPIO_LED_PIN)
    // Ask the wifi "driver" to set the GPIO on or off
    cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, led_on);
#endif
}


// UART settings
#define UART_ID uart0
#define BAUD_RATE 31250
#define UART_TX_PIN 0
#define UART_RX_PIN 1
#define GPIO_FUNC_UART 2

// U153 / M5Stack 8Encoder (U153) register map (from I2C protocol spec):
//   0x00-0x1F: Encoder counters (4 bytes per channel, channels 0-7)
//   0x50-0x57: Button status (1 byte per channel; 1=pressed, 0=released)
//   0x60:      Toggle switch status (0 or 1)
//   0x70-0x87: LED RGB data (3 bytes per LED, LEDs 0-7)
#define U153_I2C_ADDR       0x41
#define REG_ENCODER_START   0x00
#define REG_BUTTON_START    0x50
#define REG_SWITCH_BYTE     0x60
#define REG_LED_START       0x70
#define I2C_PORT            i2c0
#define PIN_SDA             8
#define PIN_SCL             9

// Timing configurations for click detection (in milliseconds)
#define DEBOUNCE_MS         30
#define DOUBLE_CLICK_WINDOW 300
#define FLASH_DURATION_MS   150

extern int midi_current_step;
extern int style_section;
extern int active_strum_pattern;
extern int active_neck_pos;
extern int basic_chord;
extern int advanced_chord;
extern int style_group;

extern uint8_t sample_drum_velocity;
extern uint8_t sample_bass_velocity;
extern uint8_t sample_chord_velocity;
extern uint8_t midi_guitar_velocity;

extern bool style_started;
extern bool enable_ample_guitar;
extern bool enable_midi_drums;
extern bool enable_seqtrak;
extern bool enable_chord_track;
extern bool enable_bass_track;
extern bool enable_modx;
extern bool enable_sp404mk2;
extern bool enable_mpc_sample;
extern bool enable_mpx_looper;
extern bool enable_nanobox_tangerine;
extern bool enable_wav_trigger_pro;
extern bool style_end_requested;
extern bool preferences_changed;

extern uint8_t but4; 
extern uint8_t mbut0;
extern uint8_t dpad_down;
extern uint8_t logo;
extern uint8_t starpower;
extern uint8_t orange;
extern bool joy_up; 
extern uint8_t joystick_up;

static uint32_t old_p1 = 0;
static uint32_t old_p2 = 0;
static uint32_t old_p3 = 0;
static uint32_t old_p4 = 0;

// 128-bit bitmask tracking currently held MIDI notes (one bit per note number).
static uint32_t held_notes_mask[4] = {0};
static int held_note_count = 0;

// MIDI running-status parser state (persists across tuh_midi_rx_cb invocations).
static uint8_t midi_running_status = 0;
static uint8_t midi_data0 = 0;
static int     midi_data_count = 0;

uint8_t device_addr = 255;

void send_ble_midi(uint8_t* midi_data, int len);
void midi_task(void);
void midi_start_stop(bool start);
void midi_send_note(uint8_t command, uint8_t note, uint8_t velocity);
void midi_send_chord_note(uint8_t note, uint8_t velocity);
void midi_send_program_change(uint8_t command, uint8_t code);
void midi_send_control_change(uint8_t command, uint8_t controller, uint8_t value);
void midi_play_chord(bool on, uint8_t p1, uint8_t p2, uint8_t p3);
void midi_play_slash_chord(bool on, uint8_t p1, uint8_t p2, uint8_t p3, uint8_t p4);
void midi_ketron_arr(uint8_t code, bool on);
void midi_ketron_footsw(uint8_t code, bool on);
void midi_seqtrak_pattern(uint8_t pattern);
void midi_seqtrak_mute(uint8_t track, bool mute);
void midi_seqtrak_key(uint8_t key);
void midi_seqtrak_tempo(int tempo);
void midi_seqtrak_arp();
void midi_seqtrak_arp_octave(uint8_t track, int octave);
void midi_modx_tempo(int tempo);
void midi_modx_key(uint8_t key);
void midi_modx_octave(uint8_t octave);
void midi_modx_arp(bool on);
void midi_modx_arp_hold(uint8_t part, bool on);
void midi_modx_arp_realtime(uint8_t part, bool on);
void midi_modx_arp_octave(uint8_t octave);
void dream_set_delay(int tempo);
void sampler_midi_note(uint8_t command, uint8_t note, uint8_t velocity);
void config_mpc_sample();
void sampler_trigger_loop();
void mpc_trigger_loop();
void mpx_trigger_loop();
void sp404_trigger_loop();
void config_nanobox_tangerine();
void config_mpx_looper();

uint8_t get_arp_template(void);
void midi_n_stream_write(uint8_t itf, uint8_t cable_num, const uint8_t *buffer, uint32_t bufsize);

enum {
	// Balanced size: enough to batch multiple MIDI packets per callback while keeping stack usage small.
	HOST_MIDI_RX_BUFFER_SIZE = 48,
};

bool repeating_timer_callback(__unused struct repeating_timer *t) {
    //printf("Repeat at %lld\n", time_us_64());
	static uint8_t previous = 0;
	uint8_t current = 60;
	
	//if (previous) midi_send_note(0x89, previous, 0);
	//midi_send_note(0x99, current, 100);	
	//board_millis();	
	previous = current;
    return true;
}

// Helper functions for I2C communication

bool u153_read_registers(uint8_t reg_addr, uint8_t *buffer, size_t length) {
    int bytes_written = i2c_write_blocking(I2C_PORT, U153_I2C_ADDR, &reg_addr, 1, true);
    if (bytes_written < 0) return false;
    int bytes_read = i2c_read_blocking(I2C_PORT, U153_I2C_ADDR, buffer, length, false);
    return (bytes_read == (int)length);
}

bool u153_update_all_leds(uint8_t *led_rgb_array) {
    uint8_t tx_buffer[25];
    tx_buffer[0] = REG_LED_START;
    memcpy(&tx_buffer[1], led_rgb_array, 24);
    int bytes_written = i2c_write_blocking(I2C_PORT, U153_I2C_ADDR, tx_buffer, sizeof(tx_buffer), false);
    return (bytes_written == sizeof(tx_buffer));
}

// core1: handle host events
void core1_main() {
	//sleep_ms(10);
	tuh_init(BOARD_TUH_RHPORT);

	while (true) {
		tuh_task(); // tinyusb host task
	}
}

int main() {
	set_sys_clock_khz(120000, true);
	sleep_ms(10);
	
    stdio_init_all();		

    int rc = pico_led_init();
    hard_assert(rc == PICO_OK);
		
    board_init();	
	
	multicore_reset_core1();
	multicore_launch_core1(core1_main);	
	flash_safe_execute_core_init();	

	tud_init(BOARD_TUD_RHPORT);			
	bluetooth_init();

    //struct repeating_timer timer;	
    //add_repeating_timer_ms(500, repeating_timer_callback, NULL, &timer);
	
	async_timer_init();
	looper_schedule_step_timer();
    note_scheduler_init();
	
	// set UART0 speed.
	uart_init(UART_ID, BAUD_RATE);
	gpio_set_function(UART_TX_PIN, GPIO_FUNC_UART);
	gpio_set_function(UART_RX_PIN, GPIO_FUNC_UART);
	uart_set_fifo_enabled(UART_ID, true);
	uart_set_translate_crlf(UART_ID, false);
	sleep_ms(500);	

	// set I2C0
    i2c_init(I2C_PORT, 400 * 1000);
    gpio_set_function(PIN_SDA, GPIO_FUNC_I2C);
    gpio_set_function(PIN_SCL, GPIO_FUNC_I2C);
    gpio_pull_up(PIN_SDA);
    gpio_pull_up(PIN_SCL);
    sleep_ms(2000);	
	
    // Hardware polling buffers
    uint8_t enc_raw_data[32];
    int32_t hardware_encoders[8] = {0};
    int32_t last_hardware_encoders[8] = {0};
    uint8_t button_states[8] = {0};
    uint8_t last_button_states[8] = {0};
    uint8_t switch_state = 0;
    uint8_t last_switch_state = 0xFF;

    // Software operational arrays
    int32_t layer_a_values[8] = {0};
    int32_t layer_b_values[8] = {0};
    uint8_t led_states[24] = {0}; 

    // Click tracking structures for all 8 buttons
    uint32_t last_press_time[8] = {0};
    uint32_t last_release_time[8] = {0};
    uint32_t flash_end_time[8] = {0};	

    // Startup LED test: briefly light all LEDs at low brightness to confirm I2C is alive
    memset(led_states, 20, sizeof(led_states)); // 20/255 = dim white
    u153_update_all_leds(led_states);
    sleep_ms(500);
    memset(led_states, 0, sizeof(led_states));
    u153_update_all_leds(led_states);

    while (true) {
		tud_task(); // tinyusb device task		
		
		if (enable_midi_drums) cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, false);			
		
		while (tud_midi_available()) {
			uint8_t buffer[4] = {0};			
			tud_midi_packet_read(buffer);
			
			if (device_addr != 255) {
				tuh_midi_stream_write(device_addr, 0, buffer, 4);
				tuh_midi_write_flush(device_addr);
			}
			
			uart_write_blocking(UART_ID, buffer, 4);
			uart_tx_wait_blocking(UART_ID); 	
	
			cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, true);				
		}	
		
		note_scheduler_dispatch_pending();

		if (preferences_changed) {
			preferences_changed = false;
			storage_store_preferences();
			
			watchdog_enable(1, 1); 			// force reboot
			while(1); 
		}
		
		// Handle 8 Encoders and LEDs on I2C0
		
        uint32_t now = to_ms_since_boot(get_absolute_time());

        bool enc_success = u153_read_registers(REG_ENCODER_START, enc_raw_data, 32);
        bool btn_success = u153_read_registers(REG_BUTTON_START, button_states, sizeof(button_states));
        bool sw_success  = u153_read_registers(REG_SWITCH_BYTE, &switch_state, 1);

        if (enc_success && btn_success && sw_success) {
            bool print_update = false;

            // Handle Global Layer Switch Changed
            if (switch_state != last_switch_state) {
                //printf("\n--- Layer Changed to: %s ---\n", (switch_state == 0) ? "LAYER A (Green)" : "LAYER B (Red)");
                last_switch_state = switch_state;
                print_update = true;
            }

            // Core loop for each of the 8 encoder channels
            for (int i = 0; i < 8; i++) {
                // EXTRACT ENCODER REVOLUTION DELTAS
                int base_idx = i * 4;
                hardware_encoders[i] = (int32_t)(
                    enc_raw_data[base_idx]         |
                    (enc_raw_data[base_idx + 1] << 8)  |
                    (enc_raw_data[base_idx + 2] << 16) |
                    (enc_raw_data[base_idx + 3] << 24)
                );

                int32_t delta = hardware_encoders[i] - last_hardware_encoders[i];
                if (delta != 0) {
                    print_update = true;
                    last_hardware_encoders[i] = hardware_encoders[i];
                    
                    if (switch_state == 0) {
                        layer_a_values[i] += delta;
                        if (layer_a_values[i] < 0) layer_a_values[i] = 0;
                        if (layer_a_values[i] > 100) layer_a_values[i] = 100;
                    } else {
                        layer_b_values[i] += delta;
                        if (layer_b_values[i] < 0) layer_b_values[i] = 0;
                        if (layer_b_values[i] > 100) layer_b_values[i] = 100;
                    }
                }

                // STATE MACHINE: DOUBLE CLICK RESET DETECTION
                // The 8Encoder returns 1 when a button is pressed, 0 when released.
                bool is_pressed = (button_states[i] != 0);
                bool was_pressed = (last_button_states[i] != 0);

                if (is_pressed && !was_pressed) { // Edge Trigger: Button Just Pressed
                    if ((now - last_press_time[i]) > DEBOUNCE_MS) {
                        // Check if the time since the PREVIOUS click falls within double click window
                        if ((now - last_press_time[i]) < DOUBLE_CLICK_WINDOW) {
                            // Double click verified! Reset values on active layer
                            if (switch_state == 0) layer_a_values[i] = 0;
                            else layer_b_values[i] = 0;
                            
                            flash_end_time[i] = now + FLASH_DURATION_MS; // Queue a quick flash visual
                            print_update = true;
                        }
                        last_press_time[i] = now;
                    }
                }

                // LED RENDERING MATRIX
				
                if (now < flash_end_time[i]) {
                    // Flash White confirmation on reset
                    led_states[i * 3]     = 60; // Red
                    led_states[i * 3 + 1] = 60; // Green
                    led_states[i * 3 + 2] = 60; // Blue
                } else if (is_pressed) {
                    // Standard single hold gives a Blue feedback line
                    led_states[i * 3]     = 0;   
                    led_states[i * 3 + 1] = 0;   
                    led_states[i * 3 + 2] = 60;  
                } else {
                    // Normal operational lighting (Layer A = Green, Layer B = Red)
                    if (switch_state == 0) {
                        led_states[i * 3]     = 0;
                        led_states[i * 3 + 1] = layer_a_values[i];
                        led_states[i * 3 + 2] = 0;
                    } else {
                        led_states[i * 3]     = layer_b_values[i];
                        led_states[i * 3 + 1] = 0;
                        led_states[i * 3 + 2] = 0;
                    }
                }
            }

            for (int i = 0; i < 8; i++) {
                last_button_states[i] = button_states[i];
            }
            u153_update_all_leds(led_states);

            if (print_update) {
                if (switch_state == 0) {
                    //printf("Active Layer A (Green): ");
                    //for(int i=0; i<8; i++) printf("[%d] ", layer_a_values[i]);
                } else {
                    //printf("Active Layer B (Red):   ");
                    //for(int i=0; i<8; i++) printf("[%d] ", layer_b_values[i]);
                }
                //printf("\n");
            }
        }		
    }
	
    //cancel_repeating_timer(&timer);	
}

//--------------------------------------------------------------------+
// Device callbacks
//--------------------------------------------------------------------+

void name_received_cb(tuh_xfer_t* xfer) {
    if (xfer->result == XFER_RESULT_SUCCESS) {
        uint16_t* temp_buf = (uint16_t*) xfer->buffer;
        char name[128];

        // Convert the UTF-16 USB string to a standard C-string (UTF-8/ASCII)
        // temp_buf[0] contains the length/type info, actual chars start at index 1
        uint8_t len = (uint8_t)(temp_buf[0] & 0xFF);
        uint8_t char_count = (len - 2) / 2;

        for (uint8_t i = 0; i < char_count; i++) {
            name[i] = (char)temp_buf[i + 1];
        }
        name[char_count] = '\0';
		
		// iRig Keys 2 PRO
		// MPC Sample
		// X-TOUCH MINI
		// nanobox tangerine
		
		if (name[0] == 'i' && name[1] == 'R' && name[2] == 'i' && name[3] == 'g' && name[4] == ' ' && name[5] == 'K' && name[6] == 'e' && name[7] == 'y' && name[8] == 's' && name[9] == ' ' && name[10] == '2' && name[11] == ' ' && name[12] == 'P' && name[13] == 'R' && name[14] == 'O') {		
			// Start/stop 	CC23 (0x17, 0x7F)
			// Next Style	CC22 (0x16, 0x1)
			// Prev Style	CC22 (0x16, 0x16)
			// Volume 		CCXX ((0x0C - 0x13), (0 - 7F)) 
			
			enable_nanobox_tangerine = true;									// assume nanobox tangerine is also connected by MIDI	
			config_nanobox_tangerine();
			cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, true);	
		}
		else
			
		if (name[0] == 'M' && name[1] == 'P' && name[2] == 'X' && name[3] == '8') {		
			enable_mpx_looper = true;	
			config_mpx_looper();
			cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, true);		
		}
		else
			
		if (name[0] == 'M' && name[1] == 'P' && name[2] == 'C' && name[3] == ' ' && name[4] == 'S' && name[5] == 'a' && name[6] == 'm' && name[7] == 'p' && name[8] == 'l' && name[9] == 'e') {		
			enable_mpc_sample = true;	
			cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, true);		
		}
		else
			
		if (name[0] == 'X' && name[1] == '-' && name[2] == 'T' && name[3] == 'O' && name[4] == 'U' && name[5] == 'C' && name[6] == 'H' && name[7] == ' ' && name[8] == 'M' && name[9] == 'I' && name[10] == 'N' && name[11] == 'I') {		
			// Dial press    (A) Note On/Off 0x00 - 0x07 (8 buttons)
			// Buttons       (A) Note On/Off 0x08 - 0x17 (16 buttons)
			// Dial volume   (A) CC 0xNN 0x00 - 0x7F (NN = 0x01 - 0x08)
			// Master volume (A) CC 0x09 0x00 - 0x7F
			// Master volume (B) CC 0x0A 0x00 - 0x7F			
			// Dial volume   (A) CC 0xNN 0x00 - 0x7F (NN = 0x0B - 0x12)	
			// Dial press    (B) Note On/Off 0x18 - 0x1F (8 buttons)			
			// Buttons       (B) Note On/Off 0x20 - 0x2F (16 buttons)
			
			cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, true);		
		}
		else

		if (name[0] == 'n' && name[1] == 'a' && name[2] == 'n' && name[3] == 'o' && name[4] == 'b' && name[5] == 'o' && name[6] == 'x' && name[7] == ' ' && name[8] == 't' && name[9] == 'a' && name[10] == 'n' && name[11] == 'g' && name[12] == 'e' && name[13] == 'r' && name[14] == 'i' && name[15] == 'n' && name[16] == 'e') {		
			enable_nanobox_tangerine = true;
			config_nanobox_tangerine();
			cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, true);		
		}
    }
}

void tuh_mount_cb(uint8_t daddr) {
    static uint16_t temp_buf[128]; 

    // Request the Product String (the device name)
    // 0x0409 is the Language ID for English (US)
	cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, false);	
    tuh_descriptor_get_product_string(daddr, 0x0409, temp_buf, sizeof(temp_buf), name_received_cb, 0);
}

void tuh_umount_cb(uint8_t daddr) {
	(void) daddr;
	cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, false);	
}

void tuh_midi_mount_cb(uint8_t idx, const tuh_midi_mount_cb_t* mount_cb_data) {
	(void) mount_cb_data;
	device_addr = idx;
	
	if (enable_mpc_sample) {
		config_mpc_sample();
	}
}

void tuh_midi_umount_cb(uint8_t idx) {
	(void) idx;
	device_addr = 255;
	cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, false);
}

// ─── Chord detection helpers ────────────────────────────────────────────────

static void chord_note_on(uint8_t note) {
    uint32_t bit = 1u << (note & 31);
    if (!(held_notes_mask[note >> 5] & bit)) {
        held_notes_mask[note >> 5] |= bit;
        held_note_count++;
    }
}

static void chord_note_off(uint8_t note) {
    uint32_t bit = 1u << (note & 31);
    if (held_notes_mask[note >> 5] & bit) {
        held_notes_mask[note >> 5] &= ~bit;
        if (held_note_count > 0) held_note_count--;
    }
}

// Collect unique pitch classes of all held notes into pcs[]; set *lowest to
// the lowest held MIDI note number.  Returns the number of unique pitch classes.
static int chord_collect_pcs(uint8_t pcs[12], uint8_t *lowest) {
    int count = 0;
    *lowest = 255;
    for (int n = 0; n < 128; n++) {
        if (!(held_notes_mask[n >> 5] & (1u << (n & 31)))) continue;
        if (*lowest == 255) *lowest = (uint8_t)n;
        uint8_t pc = (uint8_t)(n % 12);
        bool dup = false;
        for (int i = 0; i < count; i++) {
            if (pcs[i] == pc) { dup = true; break; }
        }
        if (!dup && count < 12) pcs[count++] = pc;
    }
    return count;
}

// Returns true when pcs[0..2] is exactly the set {root, root+i0, root+i1} mod 12.
static bool triad_match(uint8_t root, const uint8_t pcs[3], uint8_t i0, uint8_t i1) {
    uint8_t t1 = (root + i0) % 12;
    uint8_t t2 = (root + i1) % 12;
    int found = 0;
    for (int i = 0; i < 3; i++) {
        if (pcs[i] == root || pcs[i] == t1 || pcs[i] == t2) found++;
    }
    return found == 3;
}

// Analyse the currently held notes and print the detected chord whenever 3 or
// 4 notes are pressed simultaneously.  Recognised chord types:
//   3-note: major {0,4,7}, minor {0,3,7}, sus4 {0,5,7}
//   4-note: maj7  {0,4,7,11}; falls back to a slash-chord triad + bass note.
static void chord_detect(void) {
    if (held_note_count < 3) return;

    uint8_t pcs[12];
    uint8_t lowest = 255;
    int n = chord_collect_pcs(pcs, &lowest);
    if (n < 3 || n > 4) return;

    const char *chord_name = NULL;
    uint8_t chord_root = 255;

    if (n == 3) {
        // Interval tables for triad shapes
        static const uint8_t ti0[3] = { 4, 3, 5 };   // major, minor, sus4
        static const uint8_t ti1[3] = { 7, 7, 7 };
        static const char *const tn[3] = { "maj", "min", "sus4" };
        for (int t = 0; t < 3 && !chord_name; t++) {
            for (int r = 0; r < 3 && !chord_name; r++) {
                if (triad_match(pcs[r], pcs, ti0[t], ti1[t])) {
                    chord_root = pcs[r];
                    chord_name = tn[t];
                }
            }
        }
    } else { // n == 4
        // Try maj7: {root, root+4, root+7, root+11}
        for (int r = 0; r < 4 && !chord_name; r++) {
            uint8_t t1 = (pcs[r] + 4)  % 12;
            uint8_t t2 = (pcs[r] + 7)  % 12;
            uint8_t t3 = (pcs[r] + 11) % 12;
            int found = 0;
            for (int i = 0; i < 4; i++) {
                if (pcs[i] == pcs[r] || pcs[i] == t1 ||
                    pcs[i] == t2      || pcs[i] == t3) found++;
            }
            if (found == 4) { chord_root = pcs[r]; chord_name = "maj7"; }
        }
        // Fallback: try a triad within any 3-note subset (slash chord)
        if (!chord_name) {
            static const uint8_t si0[3] = { 4, 3, 5 };
            static const uint8_t si1[3] = { 7, 7, 7 };
            static const char *const sn[3] = { "maj", "min", "sus4" };
            for (int skip = 0; skip < 4 && !chord_name; skip++) {
                uint8_t sub[3]; int si = 0;
                for (int j = 0; j < 4; j++) if (j != skip) sub[si++] = pcs[j];
                for (int t = 0; t < 3 && !chord_name; t++) {
                    for (int r = 0; r < 3 && !chord_name; r++) {
                        if (triad_match(sub[r], sub, si0[t], si1[t])) {
                            chord_root = sub[r];
                            chord_name = sn[t];
                        }
                    }
                }
            }
        }
    }

    if (chord_name && lowest != 255) {
        // Map chord type to advanced_chord type nibble (pico_bluetooth.c scheme):
        //   0 = major  (also used for maj7, which has no dedicated sampler type)
        //   1 = minor
        //   2 = sus4
        uint8_t type = 0;
        if (strcmp(chord_name, "min") == 0)  type = 1;
        else if (strcmp(chord_name, "sus4") == 0) type = 2;

        // Encode advanced_chord: high byte = root (1-based), middle nibble =
        // bass (1-based), low nibble = type.  Mirrors the pico_bluetooth.c scheme.
        uint8_t root_1based = chord_root + 1;
        uint8_t bass_1based = (lowest % 12) + 1;
        advanced_chord = (root_1based * 256) + (bass_1based * 16) + type;

		if (enable_mpx_looper) 	{					
			mpx_trigger_loop();
		}
		else
		
        if (enable_mpc_sample) {
            mpc_trigger_loop();
        } 
		else 
		
		if (enable_nanobox_tangerine || enable_wav_trigger_pro) {
            sampler_trigger_loop();
        }		
		else 
		
		if (enable_sp404mk2) {
            sp404_trigger_loop();
        }
    }
}

// ────────────────────────────────────────────────────────────────────────────

void tuh_midi_rx_cb(uint8_t idx, uint32_t xferred_bytes) {
	if (xferred_bytes == 0) return;

	uint8_t buffer[HOST_MIDI_RX_BUFFER_SIZE];
	uint8_t cable_num = 0;
	uint32_t bytes_read = 0;

	while ((bytes_read = tuh_midi_stream_read(idx, &cable_num, buffer, sizeof(buffer))) > 0) 
	{	
		tud_midi_n_stream_write(idx, cable_num, buffer, bytes_read);

		if (!enable_mpx_looper) { 						// filter midi events from mpx pads				
			uart_write_blocking(UART_ID, buffer, bytes_read);
			uart_tx_wait_blocking(UART_ID);			
		}
		
		for (uint32_t i=0; i<bytes_read; i++) 
		{
			if (enable_mpc_sample || enable_sp404mk2 || enable_nanobox_tangerine || enable_wav_trigger_pro) {
				// Parse the raw MIDI byte stream to track note on/off events.
				uint8_t b = buffer[i];
				if (b & 0x80) {
					// Status byte.
					if (b >= 0xF8) {
						// Real-time message (single byte); does not affect running status.
						continue;
					}
					if (b >= 0xF0) {
						// System Common message; cancels running status per MIDI spec.
						midi_running_status = 0;
						midi_data_count = 0;
						continue;
					}
					// Channel message: update running status; reset data accumulator.
					midi_running_status = b;
					midi_data_count = 0;
				} else {
					// Data byte — only process Note On / Note Off messages.
					uint8_t cmd = midi_running_status & 0xF0;
					
					if (cmd == 0x80 || cmd == 0x90) 
					{
						if (midi_data_count == 0) {
							midi_data0 = b;        // first data byte: note number
							midi_data_count = 1;
						} else {
							// Second data byte: velocity.  Complete the message.
							uint8_t note     = midi_data0;
							uint8_t velocity = b;
							midi_data_count  = 0;  // ready for next running-status pair

							if (style_started) {
								bool note_on = (cmd == 0x90) && (velocity > 0);
								
								if (note_on) {
									chord_note_on(note);
								} else {
									chord_note_off(note);
								}
								
								chord_detect();
							}
							else 
								
							if (style_end_requested && (enable_nanobox_tangerine || enable_wav_trigger_pro)) {
								sampler_trigger_loop();
							}							
						}
					}
					else
						
					if (cmd == 0xB0) 
					{				
						if (midi_data_count == 0) {
							midi_data0 = b;        			// first data byte: cc command
							midi_data_count = 1;
						} else {						
							uint8_t cc_cmd	= midi_data0;	
							uint8_t cc_value = b;			// Second data byte: value.  Complete the message.
							midi_data_count  = 0; 			// ready for next running-status pair
							
							if (cc_cmd == 0x17 && cc_value == 0x7F) {			// data button press
							
								if (held_note_count < 3) {						// start/stop
									mbut0 = 1; logo = 0;
									midi_bluetooth_handle_data();
								
								} else {										// fill
									joy_up = true; joystick_up = 0;
									midi_bluetooth_handle_data();									
								}
							}
							else

							if (cc_cmd == 0x16) {								// data button dial
							
								if (style_started) {
									if (cc_value == 0x1) {							// next style
										dpad_down = 1; starpower = 0;	
										midi_bluetooth_handle_data();
									}
									else
										
									if (cc_value == 0x7F) {							// previous style
										dpad_down = 1; starpower = 0; orange = 0; but4 = 1;
										midi_bluetooth_handle_data();								
									}
								} else {
									
									if (cc_value == 0x1) {							// next style group
										style_group = style_group + 1;
										if (style_group > 20) style_group = 0;
									}
									else
										
									if (cc_value == 0x7F) {							// previous style group
										style_group = style_group - 1;
										if (style_group < 0) style_group = 20;								
									}

									if (enable_wav_trigger_pro) {
										sampler_midi_note(0x9F, 36 + style_group, 127);	 // select and load preset  
									} 									
									else

									if (enable_nanobox_tangerine) {
										midi_send_program_change(0xCF, style_group + 2); // select preset on channel 16 and skip both 1010 pianos	
									}																		
								}
							}
							else
								
							if (cc_cmd == 0x0C) {			// drum volume
								sample_drum_velocity = cc_value > 0 ? cc_value : 64;
							}
							else
								
							if (cc_cmd == 0x0D) {			// bass volume
								sample_bass_velocity = cc_value > 0 ? cc_value : 64;
							}
							else

							if (cc_cmd == 0x0E) {			// chord volume
								sample_chord_velocity = cc_value > 0 ? cc_value : 64;
							}
							else

							if (cc_cmd == 0x0F) {			// midi guitar volume
								midi_guitar_velocity = cc_value > 0 ? cc_value : 64;
							}							
						}						
						
					} else {
						// Other channel messages: Program Change (0xC0) and Channel
						// Pressure (0xD0) carry one data byte; all others carry two.
						uint8_t expected = ((cmd == 0xC0) || (cmd == 0xD0)) ? 1 : 2;
						midi_data_count++;
						if (midi_data_count >= expected) midi_data_count = 0;
					}
				}
			}
		}
	
		cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, true);		
	}
}

void tuh_midi_tx_cb(uint8_t idx, uint32_t xferred_bytes) {
	(void) idx;
	(void) xferred_bytes;
}


// Invoked when device is mounted
void tud_mount_cb(void)
{
  cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, true);
}

// Invoked when device is unmounted
void tud_umount_cb(void)
{
  cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, false);
}

// Invoked when usb bus is suspended
// remote_wakeup_en : if host allow us  to perform remote wakeup
// Within 7ms, device must draw an average of current less than 2.5 mA from bus
void tud_suspend_cb(bool remote_wakeup_en)
{
  (void) remote_wakeup_en;
  cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, false);
}

// Invoked when usb bus is resumed
void tud_resume_cb(void)
{
  cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, true);
}

//--------------------------------------------------------------------+
// MIDI Tasks
//--------------------------------------------------------------------+

void dream_set_delay(int tempo) {
	uint8_t rate = (60000 / tempo / 128 / 4) % 128;
	int check_sum = (rate % 128);
	check_sum = 128 - check_sum;
	if (check_sum == 128) check_sum = 0;
	
	uint8_t msg[11];	
	msg[0] = 0xF0;
	msg[1] = 0x41;
	msg[2] = 0x00;   
	msg[3] = 0x42;
	msg[4] = 0x12;
	msg[5] = 0x40; 
	msg[6] = 0x01;
	msg[7] = 0x35;
	msg[8] = rate;
	msg[9] = check_sum;	
	msg[10] = 0xF7;	
	midi_n_stream_write(0, 0, msg, 11);	

	//msg[7] = 0x3D;
	//midi_n_stream_write(0, 0, msg, 11);		
}

void midi_modx_key(uint8_t key) {
	if (!enable_modx) return;
	
	uint8_t msg[12];	
	msg[0] = 0xF0;
	msg[1] = 0x43;
	msg[2] = 0x10;   
	msg[3] = 0x7F;
	msg[4] = 0x1C;
	msg[5] = 0x0D; 
	msg[6] = 0x00;
	msg[7] = 0x00;
	msg[8] = 0x02;
	msg[9] = 0x00;	
	msg[10] = 0x40 + key;
	msg[11] = 0xF7;
	
	midi_n_stream_write(0, 0, msg, 12);	
}

void midi_modx_arp_octave(uint8_t octave) {
	if (!enable_modx) return;	
	
	uint8_t msg[12];	
	msg[0] = 0xF0;
	msg[1] = 0x43;
	msg[2] = 0x10;   
	msg[3] = 0x7F;
	msg[4] = 0x1C;
	msg[5] = 0x0D; 
	msg[6] = 0x00;
	msg[7] = 0x00;
	msg[8] = 0x02;
	msg[9] = 0x02;	
	msg[10] = 0x40 + octave;
	msg[11] = 0xF7;	
	
	midi_n_stream_write(0, 0, msg, 12);		
}

void midi_modx_arp(bool on) {
	if (!enable_modx) return;	
		
	uint8_t msg[12];	
	msg[0] = 0xF0;
	msg[1] = 0x43;
	msg[2] = 0x10;   
	msg[3] = 0x7F;
	msg[4] = 0x1C;
	msg[5] = 0x0D; 
	msg[6] = 0x06;
	msg[7] = 0x00;
	msg[8] = 0x01;
	msg[9] = 0x09;	
	msg[10] = on ? 1 : 0;
	msg[11] = 0xF7;
	
	midi_n_stream_write(0, 0, msg, 12);		
}

void midi_modx_arp_hold(uint8_t part, bool on) {
	if (!enable_modx) return;	
		
	uint8_t msg[13];	
	msg[0] = 0xF0;
	msg[1] = 0x43;
	msg[2] = 0x10;   
	msg[3] = 0x7F;
	msg[4] = 0x1C;
	msg[5] = 0x0D; 
	msg[6] = 0x10 + part;
	msg[7] = 0x00;
	msg[8] = 0x06;
	msg[9] = 0x00;	
	msg[10] = 0x00;
	msg[11] = on ? 2 : 1;
	msg[12] = 0xF7;
	
	midi_n_stream_write(0, 0, msg, 13);		
}

void midi_modx_arp_realtime(uint8_t part, bool on) {
	if (!enable_modx) return;	
		
	uint8_t msg[13];	
	msg[0] = 0xF0;
	msg[1] = 0x43;
	msg[2] = 0x10;   
	msg[3] = 0x7F;
	msg[4] = 0x1C;
	msg[5] = 0x0D; 
	msg[6] = 0x10 + part;
	msg[7] = 0x00;
	msg[8] = 0x06;
	msg[9] = 0x10;	
	msg[10] = 0x00;
	msg[11] = on ? 0 : 1;
	msg[12] = 0xF7;
	
	midi_n_stream_write(0, 0, msg, 13);		
}

void midi_modx_tempo(int tempo) {
	if (!enable_modx) return;	
	
	uint8_t msg[13];	
	msg[0] = 0xF0;
	msg[1] = 0x43;
	msg[2] = 0x10;   
	msg[3] = 0x7F;
	msg[4] = 0x1C;
	msg[5] = 0x0D; 
	msg[6] = 0x06;
	msg[7] = 0x00;
	msg[8] = 0x02;
	msg[9] = 0x1E;	
	msg[10] = (int) (tempo / 128);
	msg[11] = tempo % 128;
	msg[12] = 0xF7;
	
	midi_n_stream_write(0, 0, msg, 13);	
}

void midi_seqtrak_arp_octave(uint8_t track, int octave) {
	if (!enable_seqtrak) return;	
	
	uint8_t msg[11];	
	msg[0] = 0xF0;
	msg[1] = 0x43;
	msg[2] = 0x10;   
	msg[3] = 0x7F;
	msg[4] = 0x1C;
	msg[5] = 0x0C; 
	msg[6] = 0x31;
	msg[7] = 0x50 + track;
	msg[8] = 0x1C;
	msg[9] = 0x40 + octave;	// 0x3D - 0x43 (-3 to +3)
	msg[10] = 0xF7;
	
	midi_n_stream_write(0, 0, msg, 11);	
}

void midi_seqtrak_tempo(int tempo) {
	if (!enable_seqtrak) return;	
	
	uint8_t msg[12];	
	msg[0] = 0xF0;
	msg[1] = 0x43;
	msg[2] = 0x10;   
	msg[3] = 0x7F;
	msg[4] = 0x1C;
	msg[5] = 0x0C; 
	msg[6] = 0x30;
	msg[7] = 0x40;
	msg[8] = 0x76;
	msg[9] = (int) (tempo / 128);
	msg[10] = tempo % 128;
	msg[11] = 0xF7;
	
	midi_n_stream_write(0, 0, msg, 12);	
}

void midi_seqtrak_key(uint8_t key) {
	if (!enable_seqtrak) return;	
	
	uint8_t msg[11];	
	msg[0] = 0xF0;
	msg[1] = 0x43;
	msg[2] = 0x10;   
	msg[3] = 0x7F;
	msg[4] = 0x1C;
	msg[5] = 0x0C; 
	msg[6] = 0x30;
	msg[7] = 0x40;
	msg[8] = 0x7F;
	msg[9] = key;
	msg[10] = 0xF7;
	
	midi_n_stream_write(0, 0, msg, 11);	
}

void midi_seqtrak_mute(uint8_t track, bool mute) {
	if (!enable_seqtrak) return;	
	
	uint8_t msg[11];	
	msg[0] = 0xF0;
	msg[1] = 0x43;
	msg[2] = 0x10;   
	msg[3] = 0x7F;
	msg[4] = 0x1C;
	msg[5] = 0x0C; 
	msg[6] = 0x30;
	msg[7] = 0x50 + track;
	msg[8] = 0x29;
	msg[9] = mute ? 0x7D : 0;
	msg[10] = 0xF7;
	
	midi_n_stream_write(0, 0, msg, 11);	
}

void midi_seqtrak_pattern(uint8_t pattern) {
	if (!enable_seqtrak) return;	
	
	uint8_t msg[11];	
	msg[0] = 0xF0;
	msg[1] = 0x43;
	msg[2] = 0x10;   
	msg[3] = 0x7F;
	msg[4] = 0x1C;
	msg[5] = 0x0C; 
	msg[6] = 0x30;

	msg[8] = 0x0F;
	msg[9] = pattern;
	msg[10] = 0xF7;
				
	for (int i=0; i<7; i++) {						
		msg[7] = 0x50 + i;
		midi_n_stream_write(0, 0, msg, 11);	
	}	
}

void midi_seqtrak_arp() {
	if (!enable_seqtrak) return;	
	
	// config bass arp on track 8 and 10		
	uint8_t template = get_arp_template();
									
	midi_send_control_change(0xB7, 27, active_neck_pos == 2 ? 8 : 2);		// Use Arp type UP only for bass
	midi_send_control_change(0xB7, 28, 127); 								// Arp Gate always 200%
	midi_send_control_change(0xB7, 29, style_section % 2 == 0 ? 9 : 6); 	// Arp Speed 25% (ARRA/ARRC) or 50% (ARRB/ARRD) 
	midi_seqtrak_arp_octave(7, -2);											// set octave to -2 for bass
	
	midi_send_control_change(0xB9, 27, template);	
	midi_send_control_change(0xB9, 28, 127); 							
	midi_send_control_change(0xB9, 29, style_section % 2 == 0 ? 6 : 9); 	// flip arp speed for bass and keys			
}

uint8_t get_arp_template(void) {
	if (active_strum_pattern == -1) return 15;																							// no chord data, use pattern data
	if (active_strum_pattern == 0) 	return style_section % 2 == 0 ? 13 : 14;															// strum - use chord 
	if (active_strum_pattern == 1) 	return style_section % 2 == 0 ? 6 : 7;																// strum/bass - use random 
	if (active_strum_pattern == 2) 	return style_section % 2 == 0 ? 2 : 3;																// arp1 - use up 
	if (active_strum_pattern == 3) 	return style_section % 2 == 0 ? 4 : 5;																// arp2 - use down
	if (active_strum_pattern == 4) 	return style_section % 2 == 0 ? (active_neck_pos == 1 ? 8 : 9) : (active_neck_pos == 2 ? 10: 11);	// arp3 - use up/down
	return 15;	
}

void sampler_midi_note(uint8_t command, uint8_t note, uint8_t velocity) {
	uint8_t msg[3];	
	
	msg[0] = command;
	msg[1] = note;
	msg[2] = velocity;   
		
	midi_n_stream_write(0, 0, msg, 3);		
}

void midi_send_note(uint8_t command, uint8_t note, uint8_t velocity) {
	uint8_t channel = 0;	
	
	if (enable_wav_trigger_pro) {
		//return;
	}
	else

	if (enable_seqtrak) {
		channel = 8;
	}
	else
		
	if (enable_nanobox_tangerine) {	
		channel = 0;
	}
	else
		
	if (enable_sp404mk2) {
		channel = 14;
	}
	else
		
	if (enable_mpx_looper) {
		channel = 5;
	}
	
	uint8_t msg[3];	
	
	msg[0] = command + channel;
	msg[1] = note;
	
	if (!enable_nanobox_tangerine) {									// tangerine only uses velocity to trigger or not
		msg[2] = (uint8_t) (velocity * (midi_guitar_velocity / 127)); 
	} else {
		msg[2] = (uint8_t) (velocity);	
	}
		
	midi_n_stream_write(0, 0, msg, 3);			
}

void midi_send_control_change(uint8_t command, uint8_t controller, uint8_t value) {
	uint8_t msg[3];	
	
	msg[0] = command;
	msg[1] = controller;
	msg[2] = value;   
		
	midi_n_stream_write(0, 0, msg, 3);			
}

void midi_send_program_change(uint8_t command, uint8_t code) {
	uint8_t msg[2];	
	
	msg[0] = command;
	msg[1] = code;
		
	midi_n_stream_write(0, 0, msg, 2);		
}

void midi_start_stop(bool start) {
	uint8_t msg[1];	
	msg[0] = start ? 0xFA : 0xFC;	

	midi_n_stream_write(0, 0, msg, 1);	
	msg[0] = 0xF8;	
	midi_n_stream_write(0, 0, msg, 1);			
}

void midi_yamaha_start_stop(int8_t code, bool on) {
	uint8_t msg[6];	
	msg[0] = 0xF0;
	msg[1] = 0x43; 
	msg[2] = 0x60;
	msg[3] = code; 
	msg[4] = on ? 0x7F : 0x00;	
	msg[5] = 0xF7;

	midi_n_stream_write(0, 0, msg, 6);	
}

void midi_yamaha_arr(uint8_t code, bool on) {
	if (enable_seqtrak) return;
	
	uint8_t msg[7];	
	msg[0] = 0xF0;
	msg[1] = 0x43;
	msg[2] = 0x7E;   
	msg[3] = 0x00;
	msg[4] = code; 
	msg[5] = on ? 0x7F : 0x00;
	msg[6] = 0xF7;

	midi_n_stream_write(0, 0, msg, 7);	
}

void midi_ketron_arr(uint8_t code, bool on) {
	if (enable_seqtrak) return;
	
	uint8_t msg[8];	
	msg[0] = 0xF0;
	msg[1] = 0x26;
	msg[2] = 0x79;   
	msg[3] = 0x05;
	msg[4] = 0x00;
	msg[5] = code; 
	msg[6] = on ? 0x7F : 0x00;
	msg[7] = 0xF7;

	midi_n_stream_write(0, 0, msg, 8);	
}

void midi_ketron_footsw(uint8_t code, bool on) {
	if (enable_seqtrak) return;
	
	uint8_t msg[8];		
	msg[0] = 0xF0;
	msg[1] = 0x26;
	msg[2] = 0x7C;   
	msg[3] = 0x05;
	msg[4] = 0x01;
	msg[5] = 0x55 + code; 
	msg[6] = on ? 0x7F : 0x00;
	msg[7] = 0xF7;

	midi_n_stream_write(0, 0, msg, 8);	
}

void midi_send_chord_note(uint8_t note, uint8_t velocity) {
	uint8_t command = 0x90;
	uint8_t msg[3];	
	
	msg[0] = command + 3;
	msg[1] = note;
	msg[2] = velocity;   

	if (enable_seqtrak) 
	{
		if (enable_bass_track) {
			msg[0] = command + 7;						// AWM2 Synth (CH8)
			midi_n_stream_write(0, 0, msg, 3);	
		}
		
		if (enable_chord_track) {
			msg[0] = command + 9;						// DX Synth (CH10)
			midi_n_stream_write(0, 0, msg, 3);	
		}				
		
	} else {
		
		if (!enable_mpc_sample && !enable_sp404mk2 && !enable_mpx_looper && !enable_wav_trigger_pro) {
			midi_n_stream_write(0, 0, msg, 3);	// CH 1

			if (!enable_ample_guitar && !enable_modx && active_strum_pattern != 0 && active_strum_pattern != 1) {	// MIDI arpeggios only
				msg[0] = command + 2;
				msg[1] = (note % 12) + 48;
				midi_n_stream_write(0, 0, msg, 3);	// CH 3	

				msg[0] = command + 1;
				midi_n_stream_write(0, 0, msg, 3);	// CH 2		
			}
		}
	}	
}

void midi_play_chord(bool on, uint8_t p1, uint8_t p2, uint8_t p3) {
	
	if (!enable_ample_guitar || (enable_ample_guitar && active_strum_pattern == 0))
	{
		if (on) {
			
			if (enable_ample_guitar || enable_modx) {	// squeeze into C1 - B2
				p1 = (p1 % 12) + 36;
				p2 = (p2 % 12) + ((p2  % 12) <  (p1 % 12) ? 48 : 36);
				p3 = (p3 % 12) + ((p3  % 12) <  (p1 % 12) ? 48 : 36);
			}
			
			if (enable_seqtrak && basic_chord > 0 && (active_strum_pattern == 0 || style_started)) {	// sampler only with strum mode or full band
				uint8_t msg[3] = {0x9A, basic_chord + 59, 127};
				midi_n_stream_write(0, 0, msg, 3);
			}
			
			midi_send_chord_note( p1, enable_ample_guitar ? 100 : 127);
			midi_send_chord_note( p2, enable_ample_guitar ? 100 : 127);
			midi_send_chord_note( p3, enable_ample_guitar ? 100 : 127);		
			
			old_p1 = p1;
			old_p2 = p2;
			old_p3 = p3;						
		
			
		} else {
			midi_send_chord_note( old_p1, 0);
			midi_send_chord_note( old_p2, 0);
			midi_send_chord_note( old_p3, 0);
			midi_send_chord_note( old_p4, 0);

			if (enable_seqtrak && basic_chord > 0 && (active_strum_pattern == 0 || style_started)) {
				uint8_t msg[3] = {0x8A, basic_chord + 59, 0};
				midi_n_stream_write(0, 0, msg, 3);
			}				
		}
	}
}

void midi_play_slash_chord(bool on, uint8_t p1, uint8_t p2, uint8_t p3, uint8_t p4)  {	

	if (!enable_ample_guitar || (enable_ample_guitar && active_strum_pattern == 0))
	{	
		if (on) {

			if (enable_ample_guitar || enable_modx) {	// squeeze into C1 - B2
				p1 = (p1 % 12) + 36;
				p2 = (p2 % 12) + ((p2  % 12) <  (p1 % 12) ? 48 : 36);
				p3 = (p3 % 12) + ((p3  % 12) <  (p1 % 12) ? 48 : 36);
				p4 = (p4 % 12) + ((p4  % 12) <  (p1 % 12) ? 48 : 36);
			}
			
			if (enable_seqtrak && basic_chord > 0 && (active_strum_pattern == 0 || style_started)) {
				uint8_t msg[3] = {0x9A, basic_chord + 59, 127};
				midi_n_stream_write(0, 0, msg, 3);
			}			
			
			midi_send_chord_note( p1, enable_ample_guitar ? 100 : 127);
			midi_send_chord_note( p2, enable_ample_guitar ? 100 : 127);
			midi_send_chord_note( p3, enable_ample_guitar ? 100 : 127);
			midi_send_chord_note( p4, enable_ample_guitar ? 100 : 127);	

			old_p1 = p1;
			old_p2 = p2;
			old_p3 = p3;		
			old_p4 = p4;
			
			
		} else {
			midi_send_chord_note( old_p1, 0);
			midi_send_chord_note( old_p2, 0);
			midi_send_chord_note( old_p3, 0);		
			midi_send_chord_note( old_p4, 0);	

			if (enable_seqtrak && basic_chord > 0 && (active_strum_pattern == 0 || style_started)) {
				uint8_t msg[3] = {0x8A, basic_chord + 59, 0};
				midi_n_stream_write(0, 0, msg, 3);
			}				
		}
	}
}

void midi_n_stream_write(uint8_t itf, uint8_t cable_num, const uint8_t *buffer, uint32_t bufsize) {
	tud_midi_n_stream_write(itf, cable_num, buffer, bufsize);
	
	if (!enable_wav_trigger_pro || (enable_wav_trigger_pro && buffer[0] < 0xA0)) 
	{
		if (device_addr != 255) {
			tuh_midi_stream_write(device_addr, cable_num, buffer, bufsize);
			tuh_midi_write_flush(device_addr);
		}
	}

	uart_write_blocking(UART_ID, buffer, bufsize);
	uart_tx_wait_blocking(UART_ID);
}