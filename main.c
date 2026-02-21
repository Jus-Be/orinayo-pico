/**
 * Copyright (c) 2020 Raspberry Pi (Trading) Ltd.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stdio.h>
#include "pico/cyw43_arch.h"
#include "pico/multicore.h"
#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include "pico/binary_info.h"
#include "bsp/board.h"
#include "tusb.h"
#include "pico_bluetooth.h"
#include "async_timer.h"
#include "storage.h"
#include "looper.h"
#include "m5audio.h"
#include "note_scheduler.h"
#include "pico/stdlib.h"

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

extern int style_section;
extern int active_strum_pattern;
extern int active_neck_pos;
extern int basic_chord;

extern bool style_started;
extern bool enable_ample_guitar;
extern bool enable_midi_drums;
extern bool enable_seqtrak;
extern bool enable_chord_track;
extern bool enable_bass_track;
extern bool enable_modx;

static uint32_t old_p1 = 0;
static uint32_t old_p2 = 0;
static uint32_t old_p3 = 0;
static uint32_t old_p4 = 0;

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

uint8_t get_arp_template(void);
uint32_t midi_n_stream_write(uint8_t itf, uint8_t cable_num, const uint8_t *buffer, uint32_t bufsize);



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

int main() {
    int rc = pico_led_init();
    hard_assert(rc == PICO_OK);
	
    board_init();
    tusb_init();
    stdio_init_all();
	
	sleep_ms(1000);		
	bluetooth_init();
	
	tud_task();	
	storage_load_tracks();		

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

	// setup UART1 
	m5audio_init();
	
	//m5audio_set_play_mode(M5AUDIO_PLAY_MODE_SINGLE_LOOP);
	//sleep_ms(1000);		
	//m5audio_play();
	
	m5audio_repeat_at_time(0, 0, 0, 8);
	
	
    while (true) {
		tud_task(); // tinyusb device task
		
		if (enable_midi_drums) cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, false);			
		
		while (tud_midi_available()) {
			uint8_t packet[4] = {0};
			
			tud_midi_packet_read(packet);
			
			uint8_t status = packet[1];
			uint8_t channel = status & 0x0F;
			uint8_t message = status & 0xF0;
			
			if (enable_midi_drums) cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, true);				
		}	
		
		note_scheduler_dispatch_pending();				
    }
	
    //cancel_repeating_timer(&timer);	
}

//--------------------------------------------------------------------+
// Device callbacks
//--------------------------------------------------------------------+

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

void midi_send_note(uint8_t command, uint8_t note, uint8_t velocity) {
	uint8_t channel = 0;	
	if (enable_seqtrak) channel = 8;
	
	uint8_t msg[3];	
	
	msg[0] = command + channel;
	msg[1] = note;
	msg[2] = velocity;   
		
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
		midi_n_stream_write(0, 0, msg, 3);	// CH 4

		if (!enable_ample_guitar && !enable_seqtrak && !enable_modx && active_strum_pattern != 0 && active_strum_pattern != 1) {	// MIDI arpeggios only
			msg[0] = command + 2;
			msg[1] = (note % 12) + 48;
			midi_n_stream_write(0, 0, msg, 3);	// CH 3	

			msg[0] = command + 1;
			midi_n_stream_write(0, 0, msg, 3);	// CH 2		
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
			
			midi_send_chord_note( p1, enable_ample_guitar ? 100 : 127);
			midi_send_chord_note( p2, enable_ample_guitar ? 100 : 127);
			midi_send_chord_note( p3, enable_ample_guitar ? 100 : 127);		
			
			old_p1 = p1;
			old_p2 = p2;
			old_p3 = p3;			

			if (enable_seqtrak && basic_chord > 0 && (active_strum_pattern == 0 || style_started)) {	// sampler only with strum mode or full band
				uint8_t msg[3] = {0x9A, basic_chord + 59, 127};
				midi_n_stream_write(0, 0, msg, 3);
			}			
			
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
			
			midi_send_chord_note( p1, enable_ample_guitar ? 100 : 127);
			midi_send_chord_note( p2, enable_ample_guitar ? 100 : 127);
			midi_send_chord_note( p3, enable_ample_guitar ? 100 : 127);
			midi_send_chord_note( p4, enable_ample_guitar ? 100 : 127);	
			
			old_p1 = p1;
			old_p2 = p2;
			old_p3 = p3;		
			old_p4 = p4;

			if (enable_seqtrak && basic_chord > 0 && (active_strum_pattern == 0 || style_started)) {
				uint8_t msg[3] = {0x9A, basic_chord + 59, 127};
				midi_n_stream_write(0, 0, msg, 3);
			}
			
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

uint32_t midi_n_stream_write(uint8_t itf, uint8_t cable_num, const uint8_t *buffer, uint32_t bufsize) {
	tud_midi_n_stream_write(itf, cable_num, buffer, bufsize);
	
	for (int i=0; i<bufsize; i++) {
		while (!uart_is_writable(UART_ID)){ }	
		uart_putc(UART_ID, buffer[i]);		
	}
}