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

extern bool enable_ample_guitar;
extern int active_strum_pattern;

static uint32_t old_p1 = 0;
static uint32_t old_p2 = 0;
static uint32_t old_p3 = 0;
static uint32_t old_p4 = 0;

void midi_task(void);
void midi_send_note(uint8_t command, uint8_t note, uint8_t velocity);
void midi_send_program_change(uint8_t command, uint8_t code);
void midi_play_chord(bool on, uint8_t p1, uint8_t p2, uint8_t p3);
void midi_play_slash_chord(bool on, uint8_t p1, uint8_t p2, uint8_t p3, uint8_t p4);
void midi_ketron_arr(uint8_t code, bool on);
void midi_ketron_footsw(uint8_t code, bool on);

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
	
	sleep_ms(1000);		
	bluetooth_init();
	
	tud_task();
	midi_send_program_change(0xC0, 25);	// jazz guitar on channel 1
	midi_send_program_change(0xC3, 89);	// warm pad on channel 4 (chords)
	

    struct repeating_timer timer;	
    add_repeating_timer_ms(500, repeating_timer_callback, NULL, &timer);
	
    while (true) {
		tud_task(); // tinyusb device task					
    }
	
    cancel_repeating_timer(&timer);	
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

void midi_send_note(uint8_t command, uint8_t note, uint8_t velocity)
{
	uint8_t msg[3];	
	
	msg[0] = command;
	msg[1] = note;
	msg[2] = velocity;   
	
	tud_midi_n_stream_write(0, 0, msg, 3);	
}

void midi_send_program_change(uint8_t command, uint8_t code)
{
	uint8_t msg[2];	
	
	msg[0] = command;
	msg[1] = code;
	
	tud_midi_n_stream_write(0, 0, msg, 2);	
}

void midi_yamaha_start_stop(int8_t code, bool on)
{
	uint8_t msg[6];	
	msg[0] = 0xF0;
	msg[1] = 0x43; 
	msg[2] = 0x60;
	msg[3] = code; 
	msg[4] = on ? 0x7F : 0x00;	
	msg[5] = 0xF7;
	
	tud_midi_n_stream_write(0, 0, msg, 6);	
}

void midi_yamaha_arr(uint8_t code, bool on)
{
	uint8_t msg[7];	
	msg[0] = 0xF0;
	msg[1] = 0x43;
	msg[2] = 0x7E;   
	msg[3] = 0x00;
	msg[4] = code; 
	msg[5] = on ? 0x7F : 0x00;
	msg[6] = 0xF7;
	
	tud_midi_n_stream_write(0, 0, msg, 7);	
}

void midi_ketron_arr(uint8_t code, bool on)
{
	uint8_t msg[8];	
	msg[0] = 0xF0;
	msg[1] = 0x26;
	msg[2] = 0x79;   
	msg[3] = 0x05;
	msg[4] = 0x00;
	msg[5] = code; 
	msg[6] = on ? 0x7F : 0x00;
	msg[7] = 0xF7;
	
	tud_midi_n_stream_write(0, 0, msg, 8);	
}

void midi_ketron_footsw(uint8_t code, bool on)
{
	uint8_t msg[8];		
	msg[0] = 0xF0;
	msg[1] = 0x26;
	msg[2] = 0x7C;   
	msg[3] = 0x05;
	msg[4] = 0x01;
	msg[5] = 0x55 + code; 
	msg[6] = on ? 0x7F : 0x00;
	msg[7] = 0xF7;
	
	tud_midi_n_stream_write(0, 0, msg, 8);	
}
void midi_play_chord(bool on, uint8_t p1, uint8_t p2, uint8_t p3) 
{
	if (!enable_ample_guitar || (enable_ample_guitar && active_strum_pattern == 0))
	{
		if (on) {
			
			if (enable_ample_guitar) {	// squeeze into C1 - B2
				p1 = (p1 % 12) + 36;
				p2 = (p2 % 12) + ((p2  % 12) <  (p1 % 12) ? 48 : 36);
				p3 = (p3 % 12) + ((p3  % 12) <  (p1 % 12) ? 48 : 36);
			}
			
			midi_send_note(0x93, p1, enable_ample_guitar ? 127 : 32);
			midi_send_note(0x93, p2, enable_ample_guitar ? 127 : 32);
			midi_send_note(0x93, p3, enable_ample_guitar ? 127 : 32);		
			
			old_p1 = p1;
			old_p2 = p2;
			old_p3 = p3;
			
		} else {
			midi_send_note(0x83, old_p1, 0);
			midi_send_note(0x83, old_p2, 0);
			midi_send_note(0x83, old_p3, 0);
			midi_send_note(0x83, old_p4, 0);		
		}
	}
}

void midi_play_slash_chord(bool on, uint8_t p1, uint8_t p2, uint8_t p3, uint8_t p4)  
{
	if (!enable_ample_guitar || (enable_ample_guitar && active_strum_pattern == 0))
	{	
		if (on) {

			if (enable_ample_guitar) {	// squeeze into C1 - B2
				p1 = (p1 % 12) + 36;
				p2 = (p2 % 12) + ((p2  % 12) <  (p1 % 12) ? 48 : 36);
				p3 = (p3 % 12) + ((p3  % 12) <  (p1 % 12) ? 48 : 36);
				p4 = (p4 % 12) + ((p4  % 12) <  (p1 % 12) ? 48 : 36);
			}
			
			midi_send_note(0x93, p1, enable_ample_guitar ? 127 : 32);
			midi_send_note(0x93, p2, enable_ample_guitar ? 127 : 32);
			midi_send_note(0x93, p3, enable_ample_guitar ? 127 : 32);
			midi_send_note(0x93, p4, enable_ample_guitar ? 127 : 32);	
			
			old_p1 = p1;
			old_p2 = p2;
			old_p3 = p3;		
			old_p4 = p4;				
		} else {
			midi_send_note(0x83, old_p1, 0);
			midi_send_note(0x83, old_p2, 0);
			midi_send_note(0x83, old_p3, 0);		
			midi_send_note(0x83, old_p4, 0);			
		}
	}
}