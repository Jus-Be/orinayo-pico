#include "pico_bluetooth.h"

#include <stddef.h>
#include <string.h>

#include <bt/uni_bt.h>
#include <btstack.h>
#include <controller/uni_gamepad.h>
#include <pico/cyw43_arch.h>
#include <pico/time.h>
#include <uni.h>
#include <uni_hid_device.h>

#include "debug.h"
#include "sdkconfig.h"
#include "button.h"
#include "looper.h"
#include "storage.h"
#include "ghost_note.h"

#ifndef CONFIG_BLUEPAD32_PLATFORM_CUSTOM
#error "Pico W must use BLUEPAD32_PLATFORM_CUSTOM"
#endif

extern looper_status_t looper_status;

bool style_started = false;
bool enable_style_play = true;
bool enable_ample_guitar = false;
bool enable_midi_drums = false;
int active_strum_pattern = 0;	
int active_neck_pos = 2;
int style_section = 0; 
int old_style = -1;
int transpose = 0; 

uint8_t old_midinotes[6] = {0};
uint8_t mute_midinotes[6] = {0};

void midi_send_note(uint8_t command, uint8_t note, uint8_t velocity);
void midi_send_program_change(uint8_t command, uint8_t code);
void midi_send_control_change(uint8_t command, uint8_t controller, uint8_t value);
void midi_play_chord(bool on, uint8_t p1, uint8_t p2, uint8_t p3);
void midi_play_slash_chord(bool on, uint8_t p1, uint8_t p2, uint8_t p3, uint8_t p4);
void midi_ketron_arr(uint8_t code, bool on);
void midi_ketron_footsw(uint8_t code, bool on);
void midi_yamaha_start_stop(uint8_t code, bool on);
void midi_yamaha_arr(uint8_t code, bool on);

void play_chord(bool on, bool up, uint8_t green, uint8_t red, uint8_t yellow, uint8_t blue, uint8_t orange);
void looper_handle_input_internal_clock(button_event_t event);
void looper_clear_all_tracks();
void looper_update_bpm(uint32_t bpm);

int chord_chat[12][3][6] = {
	{{ 3,  3, 2, 0, 1, 0}, {-1,  3, 5, 5, 4, 3}, {-1, -1, 3, 0, 1, 3}},
	{{-1, -1, 3, 1, 2, 1}, {-1, -1, 2, 1, 2, 0}, {-1, -1, 3, 3, 4, 1}},
	{{-1, -1, 0, 2, 3, 2}, {-1, -1, 0, 2, 3, 1}, {-1, -1, 0, 2, 3, 3}},
	{{-1, -1, 5, 3, 4, 3}, {-1, -1, 4, 3, 4, 2}, {-1, -1, 1, 3, 4, 4}},
	{{ 0,  2, 2, 1, 0, 0}, { 0,  2, 2, 0, 0, 0}, { 0,  2, 2, 2, 0, 0}},
	{{ 1,  3, 3, 2, 1, 1}, { 1,  3, 3, 1, 1, 1}, {-1, -1, 3, 3, 1, 1}},
	{{ 2,  4, 4, 3, 2, 2}, { 2,  4, 4, 2, 2, 2}, {-1, -1, 4, 4, 2, 2}},
	{{ 3,  2, 0, 0, 0, 3}, { 3,  5, 5, 3, 3, 3}, {-1, -1, 0, 0, 1, 3}},
	{{ 4,  6, 6, 5, 4, 4}, { 4,  6, 6, 4, 4, 4}, {-1, -1, 1, 1, 2, 4}},
	{{-1,  0, 2, 2, 2, 0}, {-1,  0, 2, 2, 1, 0}, {-1,  0, 2, 2, 3, 0}},
	{{-1,  1, 3, 3, 3, 1}, {-1,  1, 3, 3, 2, 1}, {-1, -1, 3, 3, 4, 1}},
	{{-1,  2, 4, 4, 4, 2}, {-1,  2, 4, 4, 3, 2}, {-1, -1, 4, 4, 5, 2}}
};

uint8_t strum_pattern[5][12][6] = {
	{{6,5,4,3,2,1}, {0,0,0,0,0,0}, {0,0,0,0,0,0}, {0,0,0,0,0,0}, {0,0,0,0,0,0}, {0,0,0,0,0,0}, {0,0,0,0,0,0}, {0,0,0,0,0,0}, {0,0,0,0,0,0}, {0,0,0,0,0,0}, {0,0,0,0,0,0}, {0,0,0,0,0,0}}, 
	{{4,3,2,1,0,0}, {3,2,1,0,0,0}, {4,3,2,1,0,0}, {4,3,2,0,0,0}, {0,0,0,0,0,0}, {0,0,0,0,0,0}, {0,0,0,0,0,0}, {0,0,0,0,0,0}, {0,0,0,0,0,0}, {0,0,0,0,0,0}, {0,0,0,0,0,0}, {0,0,0,0,0,0}}, 
	{{4,0,0,0,0,0}, {3,0,0,0,0,0}, {4,0,0,0,0,0}, {2,0,0,0,0,0}, {4,0,0,0,0,0}, {3,0,0,0,0,0}, {1,0,0,0,0,0}, {3,0,0,0,0,0}, {2,0,0,0,0,0}, {0,0,0,0,0,0}, {0,0,0,0,0,0}, {0,0,0,0,0,0}},	
	{{3,0,0,0,0,0}, {2,0,0,0,0,0}, {4,0,0,0,0,0}, {1,0,0,0,0,0}, {4,0,0,0,0,0}, {2,0,0,0,0,0}, {4,0,0,0,0,0}, {0,0,0,0,0,0}, {0,0,0,0,0,0}, {0,0,0,0,0,0}, {0,0,0,0,0,0}, {0,0,0,0,0,0}},	
	{{1,0,0,0,0,0}, {3,0,0,0,0,0}, {1,0,0,0,0,0}, {2,0,0,0,0,0}, {3,0,0,0,0,0}, {1,0,0,0,0,0}, {2,0,0,0,0,0}, {1,0,0,0,0,0}, {3,0,0,0,0,0}, {2,0,0,0,0,0}, {0,0,0,0,0,0}, {0,0,0,0,0,0}}, 
};

// Platform Overrides
static void pico_bluetooth_init(int argc, const char** argv) {
  ARG_UNUSED(argc);
  ARG_UNUSED(argv);
}

static void pico_bluetooth_on_init_complete(void) {
  // Safe to call "unsafe" functions since they are called
  // PICO_INFO("Bluetooth initialization complete.\n");

  // Delete stored BT keys for fresh pairing (helpful for initial connection)
  uni_bt_del_keys_unsafe();

  // Start scanning and autoconnect to supported controllers.
  uni_bt_start_scanning_and_autoconnect_safe();
  // PICO_INFO("Started Bluetooth scanning for new devices.\n");

  uni_property_dump_all();
  cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, true);
}

static uni_error_t pico_bluetooth_on_device_discovered(bd_addr_t addr, const char* name, uint16_t cod, uint8_t rssi) {
  // You can filter discovered devices here. Return any value different from UNI_ERROR_SUCCESS;
  // @param addr: the Bluetooth address
  // @param name: could be NULL, could be zero-length, or might contain the name.
  // @param cod: Class of Device. See "uni_bt_defines.h" for possible values.
  // @param rssi: Received Signal Strength Indicator (RSSI) measured in dBms. The higher (255) the better.

  const char* device_name = (name && strlen(name) > 0) ? name : "Unknown";
  // PICO_DEBUG("[BT] Found device: %s (%02X:%02X:%02X:%02X:%02X:%02X) CoD=0x%04X RSSI=%ddBm\n", device_name, addr[0], addr[1], addr[2], addr[3], addr[4], addr[5], cod, rssi);

  // Check if it's a Gamepad controller
  if (name && (strstr(name, "STANDARD GAMEPAD"))) {
    // PICO_INFO("Gamepad controller detected! Attempting connection...\n");
	 cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, true);
  }

  // As an example, if you want to filter out keyboards, do:
  if (((cod & UNI_BT_COD_MINOR_MASK) & UNI_BT_COD_MINOR_KEYBOARD) == UNI_BT_COD_MINOR_KEYBOARD) {
    // PICO_DEBUG("[BT] Ignoring keyboard device\n");
    return UNI_ERROR_IGNORE_DEVICE;
  }

  return UNI_ERROR_SUCCESS;
}

static void pico_bluetooth_on_device_connected(uni_hid_device_t* d) {
  // PICO_INFO("Device connected: %s (%02X:%02X:%02X:%02X:%02X:%02X)\n", d->name, d->conn.btaddr[0], d->conn.btaddr[1], d->conn.btaddr[2], d->conn.btaddr[3], d->conn.btaddr[4], d->conn.btaddr[5]);

  // Disable scanning when a device is connected to save power
  uni_bt_stop_scanning_safe();    
  // PICO_DEBUG("[BT] Stopped scanning (device connected)\n");
  
}

static void pico_bluetooth_on_device_disconnected(uni_hid_device_t* d) {
  // PICO_INFO("Device disconnected: %s (%02X:%02X:%02X:%02X:%02X:%02X)\n", d->name, d->conn.btaddr[0], d->conn.btaddr[1], d->conn.btaddr[2], d->conn.btaddr[3], d->conn.btaddr[4], d->conn.btaddr[5]);

  // Re-enable scanning when a device is disconnected
  uni_bt_start_scanning_and_autoconnect_safe();
  
  cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, true);  
  // PICO_DEBUG("[BT] Restarted scanning (device disconnected)\n");
}

static uni_error_t pico_bluetooth_on_device_ready(uni_hid_device_t* d) {
  // You can reject the connection by returning an error.
  cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, false);  
  
	midi_send_program_change(0xC0, 26);		// jazz guitar on channel 1
	midi_send_program_change(0xC3, 89);		// warm pad on channel 4 (chords) 
	midi_send_control_change(0xB3, 7, 0); 	// don't play pads by default
	
  return UNI_ERROR_SUCCESS;
}

static const uni_property_t* pico_bluetooth_get_property(uni_property_idx_t idx) {
  ARG_UNUSED(idx);
  return NULL;
}

static void pico_bluetooth_on_controller_data(uni_hid_device_t* d, uni_controller_t* ctl) {
  absolute_time_t now = get_absolute_time();
  uint64_t now_since_boot = to_us_since_boot(now);

#if IS_PICO_DEBUG
  static int count = 0;
  static absolute_time_t last_updated = 0;

  count++;

  int64_t elapsed_us = absolute_time_diff_us(last_updated, now);
  if (elapsed_us >= 1000000) {
    // PICO_DEBUG("[BT] Bluetooth data received: %u\n", count);
    last_updated = now;
    count = 0;
  }
#endif

  static uint8_t green = 0;
  static uint8_t red = 0;
  static uint8_t yellow = 0;
  static uint8_t blue = 0;
  static uint8_t orange = 0;
  static uint8_t starpower = 0;
  static uint8_t pitch = 0;
  
  static uint8_t up = 0;
  static uint8_t down = 0;
  static uint8_t left = 0;
  static uint8_t right = 0;	

  static uint8_t start = 0;
  static uint8_t menu = 0;
  static uint8_t logo = 0;
  static uint8_t config = 0;	
  
  static uint8_t joystick_up = 0;
  static uint8_t joystick_down = 0;  
  static uint8_t logo_knob_up = 0;  
  static uint8_t logo_knob_down = 0; 
    
  uint8_t but0 = (ctl->gamepad.buttons >> 0) & 0x01;
  uint8_t but1 = (ctl->gamepad.buttons >> 1) & 0x01;
  uint8_t but2 = (ctl->gamepad.buttons >> 2) & 0x01;
  uint8_t but3 = (ctl->gamepad.buttons >> 3) & 0x01;
  uint8_t but4 = (ctl->gamepad.buttons >> 4) & 0x01; 
  uint8_t but6 = (ctl->gamepad.buttons >> 6) & 0x01;   
  uint8_t but9 = (ctl->gamepad.buttons >> 9) & 0x01;

  uint8_t dpad_left = ctl->gamepad.dpad & 0x02;	
  uint8_t dpad_right = ctl->gamepad.dpad & 0x01;
  uint8_t dpad_up = ctl->gamepad.dpad & 0x04;
  uint8_t dpad_down = ctl->gamepad.dpad & 0x08;

  uint8_t mbut0 = (ctl->gamepad.misc_buttons >> 0) & 0x01;
  uint8_t mbut1 = (ctl->gamepad.misc_buttons >> 1) & 0x01;
  uint8_t mbut2 = (ctl->gamepad.misc_buttons >> 2) & 0x01;
  uint8_t mbut3 = (ctl->gamepad.misc_buttons >> 3) & 0x01;
  
  uint8_t axis_x = ctl->gamepad.axis_x / 4;
  uint8_t axis_y = ctl->gamepad.axis_y / 4;
  uint8_t axis_rx = ctl->gamepad.axis_rx / 4;
  uint8_t axis_ry = ctl->gamepad.axis_ry / 4;	

  bool joy_up = axis_y > axis_x;  
  bool joy_down = axis_x > axis_y;  
  bool knob_up = axis_ry > axis_rx; 
  bool knob_down = axis_rx > axis_ry; 
    
  switch (ctl->klass) {
    case UNI_CONTROLLER_CLASS_GAMEPAD:
		// cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, true);
	  
		// first get button state
				
		if (but1 != green) {
			green = but1;
		}
		
		if (but0 != red) {
			red = but0;
		}
		
		if (but2 != yellow) {
			yellow = but2;
		}
		
		if (but3 != blue) {
			blue = but3;
		}

		if (but4 != orange) {
			orange = but4;
		}
		
		if (but6 != pitch) {		// prev section/style
			pitch = but6;

			if (but6 && (up || down)) {	// reset transpose
				transpose = 0;
			}
			else

			if (yellow && blue) {
				active_neck_pos = 3;
			}
			else

			if (yellow && red) {
				active_neck_pos = 2;
			}
			else
				
			if (green && red) {
				active_neck_pos = 1;
				
				if (but6) {
					midi_send_program_change(0xC0, 33);	// bass guitar on channel 1
					active_strum_pattern = 2;			// arpeggios, no strumming
				}
			}
			else
				
			if (blue && orange) {
				active_strum_pattern = -1;
			}
			else			
							
			if (green) {
				active_strum_pattern = 0;
				
				if (but6 && enable_ample_guitar && active_neck_pos != 1) {
					midi_send_note(0x90, 86, 127);		// enable chord detection					
					midi_send_note(0x90, 97, 127);		// key switch for strum mode on
					midi_send_note(0xB0, 64, 1);		// hold pedal off	
				}
			}
			else
				
			if (yellow) {
				active_strum_pattern = 2;
				
				if (but6 && enable_ample_guitar && active_neck_pos != 1) {
					midi_send_note(0x90, 97, 1);		// key switch for strum mode off
					midi_send_note(0xB0, 64, 127);		// hold pedal on	
					midi_send_note(0x90, 99, 127);					
				}
			}
			else

			if (blue) {
				active_strum_pattern = 3;
				
				if (but6 && enable_ample_guitar && active_neck_pos != 1) {
					midi_send_note(0x90, 97, 1);		// key switch for strum mode off
					midi_send_note(0xB0, 64, 127);		// hold pedal on	
					midi_send_note(0x90, 99, 127);						
				}
			}				
			else

			if (red) {
				active_strum_pattern = 1;
				
				if (but6 && enable_ample_guitar && active_neck_pos != 1) {
					midi_send_note(0x90, 97, 1);		// key switch for strum mode off
					midi_send_note(0xB0, 64, 1);		// hold pedal off						
				}				
			}
			else

			if (orange) {
				active_strum_pattern = 4;
				
				if (but6 && enable_ample_guitar && active_neck_pos != 1) {
					midi_send_note(0x90, 97, 1);		// key switch for strum mode off
					midi_send_note(0xB0, 64, 127);		// hold pedal on	
					midi_send_note(0x90, 99, 127);						
				}			
			}
			else {		
				
				if (but6) {
					old_style = style_section;
					style_section--;
					if (style_section < 0) style_section = 7;

					if (enable_midi_drums && looper_status.state == LOOPER_STATE_PLAYING)	{
						ghost_parameters_t *params = ghost_note_parameters();
						params->ghost_intensity = (style_section % 2) == 0 ? 0 : 0.843;			
					}					
				}

				if (!enable_midi_drums) {				
					midi_ketron_arr(3 + (style_section % 4), but6 ? true : false);
					midi_yamaha_arr(0x10 + (style_section % 4), but6 ? true : false);					
				}
			}	
	
			break;
		}
		
		// next handle actions
		
		if (but9 != start)  {	// unused because start button clashes with axis (knob_up/knob_down)
			start = but9;		
			break;			
		}				
		
		if (dpad_left != left) { 	// Strum down
			left = dpad_left;
			
			if (dpad_left) {
				play_chord(true, false, green, red, yellow, blue, orange);
			} else {			
				if (enable_style_play) midi_play_chord(false, 0, 0, 0);	
				
				for (int n=0; n<6; n++) 
				{
					if (old_midinotes[n] > 0) {
						midi_send_note(0x80, old_midinotes[n], 0);	
						old_midinotes[n] = 0;
					}
				}
				
				if (looper_status.state == LOOPER_STATE_RECORDING || looper_status.state == LOOPER_STATE_TAP_TEMPO) {
					looper_handle_input_internal_clock(BUTTON_EVENT_CLICK_RELEASE);				
				}
			}
			
			if (!style_started) cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, !!dpad_left);			
			break;
		}		

		if (dpad_right != right) {	// strum up
			right = dpad_right;	

			if (dpad_right) {
				play_chord(true, true, green, red, yellow, blue, orange);
			} else {			
				if (enable_style_play) midi_play_chord(false, 0, 0, 0);	
				
				for (int n=0; n<6; n++) 
				{
					if (old_midinotes[n] > 0) {
						midi_send_note(0x80, old_midinotes[n], 0);						
						old_midinotes[n] = 0;						
					}					
				}

				if (looper_status.state == LOOPER_STATE_RECORDING || looper_status.state == LOOPER_STATE_TAP_TEMPO) {
					looper_handle_input_internal_clock(BUTTON_EVENT_CLICK_RELEASE);				
				}						
			}
	
			if (!style_started) cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, !!dpad_right);				
			break;
		}

		if (dpad_up != up) {	// transpose down
			up = dpad_up;

			if (dpad_up) {
				transpose--;
				if (transpose < 0) transpose = 11;
			}
			break;			
		}

		if (dpad_down != down) {	// transpose up
			down = dpad_down;
			
			if (dpad_down) {
				transpose++;
				if (transpose > 11) transpose = 0;		
			}
			break;
		}		
		
		uint8_t ketron_code;
		uint8_t yamaha_code;
		
		if (mbut0 != logo) {
			logo = mbut0;
			
			if (enable_midi_drums) 
			{
				if (mbut0) {
					
					if (looper_status.state == LOOPER_STATE_RECORDING || looper_status.state == LOOPER_STATE_TAP_TEMPO) {
						style_section = 0;						
						looper_status.state = LOOPER_STATE_PLAYING;
						
						//ghost_parameters_t *params = ghost_note_parameters();						
						//params->ghost_intensity = 0;						
						//storage_store_tracks();					
					} 
					else 
					
					if (looper_status.state == LOOPER_STATE_PLAYING) {
						looper_status.state = LOOPER_STATE_WAITING;
						enable_midi_drums = false;
					}	
				}					
			} else {
				ketron_code = 0x12;		// default start/stop
				yamaha_code = 127;					
				
				if (yellow) {				// INTRO/END-1
					ketron_code = 0x0F;		
					yamaha_code = 0x00;					
				}

				if (red) {
					ketron_code = 0x10;		// INTRO/END-2
					yamaha_code = 0x01;					
				}
				
				if (green) {
					ketron_code = 0x11;		// INTRO/END-3		
					yamaha_code = 0x02;					
				}
				
				if (blue) {
					ketron_code = 0x17;		// TO END
					yamaha_code = 0x01;					
				}
				
				if (orange) {
					ketron_code = 0x35;	// FADE	
					yamaha_code = 0x02;					
				}
				
				midi_ketron_arr(ketron_code, mbut0 ? true : false);
				
				if (!style_started) {
					if (yamaha_code != 127) {
						midi_yamaha_arr(yamaha_code, mbut0 ? true : false);	
					} 
					
					if (mbut0) midi_yamaha_start_stop(0x7A, true);
					
				} else {
					if (yamaha_code != 127) {
						midi_yamaha_arr(0x20 + yamaha_code, mbut0 ? true : false);	
					}
					
					if (mbut0) midi_yamaha_start_stop(0x7D, true);				
				}
				
				if (mbut0) style_started = !style_started;
				cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, style_started);
			}
			break;
		}		
		
		if (mbut1 != starpower) { // next style/section			
			starpower = mbut1;
			if (mbut1) old_style = style_section;

			if (green) 
			{
				if (mbut1) style_section = 0;
			}
			else
				
			if (red) {
				if (mbut1) style_section = 1;
			}
			else

			if (yellow) {
				if (mbut1) style_section = 2;
			}				
			else

			if (blue) 
			{
				if (mbut1) 
				{
					if (enable_midi_drums)	{						
						looper_status.state = LOOPER_STATE_RECORDING;
						looper_status.current_step = 0;
						break;
						
					} else {
						style_section = 3;
					}
				}
			}
			else

			if (orange) 
			{
				if (mbut1) 
				{
					if (enable_midi_drums)	{
						looper_status.state = LOOPER_STATE_TAP_TEMPO;					
						break;							
						
					} else {
						style_section = 4;
					}
				}
			}
			else 
			
			if (mbut1) {
				style_section++;
				if (style_section > 7) style_section = 0;
			}			
			
			if (enable_midi_drums)	
			{
				if (mbut1 && looper_status.state == LOOPER_STATE_PLAYING) {				
					ghost_parameters_t *params = ghost_note_parameters();
					params->ghost_intensity = (style_section % 2) == 0 ? 0 : 0.843;	
				}
				
			} else {	
				midi_ketron_arr(3 + (style_section % 4), mbut1 ? true : false);
				midi_yamaha_arr(0x10 + (style_section % 4), mbut1 ? true : false);	
			}				
			
			break;			
		}
		
		if (mbut2 != menu) {
			midi_ketron_footsw(8, mbut2 ? true : false);						// 	user defined from footswitch	
			menu = mbut2;
		}		
		
		if (mbut3 != config) {	// config/menu options
			config = mbut3;
			
			if (mbut3) 
			{
				if (green) {
					enable_style_play = !enable_style_play;						// enable/disable chords on channel 4
				}
				else
					
				if (red) {
					enable_ample_guitar = !enable_ample_guitar; 				// Ample Guitar VST mode
					midi_send_note(0x90, 97, enable_ample_guitar ? 127 : 1);	// set strum mode on by default
					midi_send_note(0x90, 86, 127);								// set chord detectt					
				}
				
				if (yellow) 
				{
					if (enable_midi_drums) {
						looper_clear_all_tracks();								// Midi drums looper
					}
					enable_midi_drums = !enable_midi_drums;
				}
				
				break;
			}
		}

		if (joy_up != joystick_up) {
			joystick_up = joy_up;

			if (enable_ample_guitar) {
				midi_send_note(0x90, 24, joy_up ? 127 : 0);						// sustain guitar notes
			} 
			else

			if (style_started) {
				midi_ketron_arr(0x07 + (style_section % 4), joy_up ? true : false);	// 	Fill
				midi_yamaha_arr(0x10 + (style_section % 4), joy_up ? true : false);				
			}
			break;
		}
		
		if (joy_down != joystick_down) {	// unused
			joystick_down = joy_down;			
			break;			
		}

		if (knob_up != logo_knob_up) {
			logo_knob_up = knob_up;	

			if (enable_ample_guitar) {
				midi_send_note(0x90, 26, knob_up ? 127 : 0);	// palm mute guitar notes
			} 
			else
				
			if (style_started) {			
				midi_ketron_arr(0x0B + (style_section % 4), knob_up ? true : false);	// 	break	
				midi_yamaha_arr(0x18, knob_up ? true : false);				
			}
			break;			
		}
		
		if (knob_down != logo_knob_down) {	// unused
			logo_knob_down = knob_down;					
			break;			
		}
				
      break;
    case UNI_CONTROLLER_CLASS_BALANCE_BOARD:
      // DO NOTHING
      break;
    case UNI_CONTROLLER_CLASS_MOUSE:
      // DO NOTHING
      break;
    case UNI_CONTROLLER_CLASS_KEYBOARD:
      // DO NOTHING
      break;
    default:
      //loge("Unsupported controller class: %d\n", ctl->klass);
      break;
  }
}


int compDown(const void *a, const void *b) {
    return (*(uint8_t *)b - *(uint8_t *)a);
}

int compUp(const void *a, const void *b) {
    return (*(uint8_t *)a - *(uint8_t *)b);
}

void play_chord(bool on, bool up, uint8_t green, uint8_t red, uint8_t yellow, uint8_t blue, uint8_t orange) {
	uint8_t chord_note = 0;
	uint8_t chord_type = 0;	
	uint8_t bass_note = 0;		
	uint8_t base = 60;	
	bool handled = false;	
	
	base = 60 + transpose;
	
	// --- F/C

	if (yellow && blue && orange && red) 
	{
		if (enable_style_play) midi_play_slash_chord(on, base - 12, base + 5, base + 9, base + 12);
		chord_note = (base + 5);	
		bass_note = base - 12;
		handled = true;		
	}
	else

	// --- G/C

	if (yellow && blue && orange && green) 
	{
		if (enable_style_play) midi_play_slash_chord(on, base - 12, base + 7, base + 11, base + 14);
		chord_note = (base + 7);	
		bass_note = base - 12;		
		handled = true;			
	}
	else

	// -- B

	if (red && yellow && blue && green) 
	{
		if (enable_style_play) midi_play_chord(on, base - 1, base + 3, base + 6);	
		chord_note = (base - 1);		
		handled = true;			
	}
	else

	if (red && yellow && green)     // Ab
	{
		if (enable_style_play) midi_play_chord(on, base - 4, base, base + 3);
		chord_note = (base - 4);		
		handled = true;		
	}
	else

	if (red && yellow && blue)     // A
	{
		if (enable_style_play) midi_play_chord(on, base - 3, base + 13, base + 16);
		chord_note = (base - 3);		
		handled = true;		
	}
	else

	if (blue && yellow && green)     // E
	{
		if (enable_style_play) midi_play_chord(on, base - 8, base + 8, base + 11);
		chord_note = (base - 8);		
		handled = true;		
	}
	else


	if (blue && red && orange)     // Eb
	{
		if (enable_style_play) midi_play_chord(on, base - 9, base + 7, base + 10);
		chord_note = (base - 9);		
		handled = true;		
	}
	else

	if (yellow && blue && orange)    // F/G
	{
		if (enable_style_play) midi_play_slash_chord(on, base - 17, base + 5, base + 9, base + 12);
		chord_note = (base + 5);
		bass_note = base - 17;			
		handled = true;		
	}
	else

	if (red && yellow)     // Bb
	{
		if (enable_style_play) midi_play_chord(on, base - 2, base + 2, base + 5);
		chord_note = (base - 2);		
		handled = true;		
	}
	else

	if (green && yellow)     // Gsus
	{
		if (enable_style_play) midi_play_chord(on, base - 5, base + 12, base + 14);
		chord_note = (base - 5);	
		chord_type = 2;
		handled = true;		
	}
	else

	if (orange && yellow)     // Csus
	{
		if (enable_style_play) midi_play_chord(on, base, base + 5, base + 7);
		chord_note = (base);	
		chord_type = 2;
		handled = true;			
	}
	else

	if (yellow && blue)    // C/E
	{
		if (enable_style_play) midi_play_slash_chord(on, base - 20, base, base + 4, base + 7);
		chord_note = (base);
		bass_note = base - 20;	
		handled = true;			
	}
	else

	if (green && red)     // G/B
	{
		if (enable_style_play) midi_play_slash_chord(on, base - 13, base + 7, base + 11, base + 14);
		chord_note = (base + 7);
		bass_note = base - 13;			
		handled = true;			
	}
	else

	if (blue && orange)     // F/A
	{
		if (enable_style_play) midi_play_slash_chord(on, base - 15, base + 5, base + 9, base + 12);
		chord_note = (base + 5);
		bass_note = base - 15;			
		handled = true;			
	}
	else

	if (green && blue)     // Em
	{
		if (enable_style_play) midi_play_chord(on, base - 8, base + 7, base + 11);
		chord_note = (base - 8);	
		chord_type = 1;		
		handled = true;			
	}
	else

	if (orange && red)   // Fm
	{
		if (enable_style_play) midi_play_chord(on, base - 7, base + 8, base + 12);
		chord_note = (base - 7);	
		chord_type = 1;
		handled = true;			
	}
	else

	if (green && orange)     // Gm
	{
		if (enable_style_play) midi_play_chord(on, base - 5, base + 10, base + 14);
		chord_note = (base - 5);	
		chord_type = 1;
		handled = true;			
	}
	else

	if (red && blue)     // D
	{
		if (enable_style_play) midi_play_chord(on, base + 2, base + 6, base + 9);
		chord_note = (base + 2);	
		handled = true;			
	}
	else

	if (yellow)    // C
	{
		if (enable_style_play) midi_play_chord(on, base, base + 4, base + 7);
		chord_note = (base);	
		handled = true;			
	}
	else

	if (blue)      // Dm
	{
		if (enable_style_play) midi_play_chord(on, base + 2, base + 5, base + 9);
		chord_note = (base + 2);	
		chord_type = 1;		
		handled = true;			
	}
	else

	if (orange)   // F
	{
		if (enable_style_play) midi_play_chord(on, base - 7, base + 9, base + 12);
		chord_note = (base - 7);	
		handled = true;			
	}
	else

	if (green)     // G
	{
		if (enable_style_play) midi_play_chord(on, base - 5, base + 11, base + 14);
		chord_note = (base - 5);			
		handled = true;			
	}
	else

	if (red)     // Am
	{
		if (enable_style_play) midi_play_chord(on, base - 3, base + 12, base + 16);
		chord_note = (base - 3);	
		chord_type = 1;			
		handled = true;			
	}
	
	int O = 12;
	int C = 0, Cs = 1, Db = 1, D = 2, Ds = 3, Eb = 3, E = 4, F = 5, Fs = 6, Gb = 6, G = 7, Gs = 8, Ab = 8, A = 9, As = 10, Bb = 10, B = 11;	
	int __6th = E +O*(active_neck_pos+2), __5th = A +O*(active_neck_pos+2), __4th = D +O*(active_neck_pos+2), __3rd = G +O*(active_neck_pos+2), __2nd = B +O*(active_neck_pos+2), __1st = E +O*(active_neck_pos+3);	
	
	int string_frets[6];
	string_frets[0] = __6th;
	string_frets[1] = __5th;
	string_frets[2] = __4th;
	string_frets[3] = __3rd;
	string_frets[4] = __2nd;
	string_frets[5] = __1st;
		
	int chord_frets[6] = {0};	
	uint8_t chord_midinotes[6] = {0};
	
	static int seq_index = 0;
	
	int string = 0;
	int notes_count = 0;
	int velocity = 100;	

	uint8_t note = 0;
	uint8_t ample_style_notes[8] = {36, 37, 39, 42, 44, 46, 49, 51};
	uint8_t ample_string_notes[6] = {47, 45, 43, 41, 40, 38};
	
	if (active_strum_pattern > -1) 
	{
		if (handled) 
		{
			if (up || active_strum_pattern == 0) 
			{										
				if (enable_ample_guitar && active_strum_pattern == 0) 
				{	
					if (style_section != old_style) {
						note = ample_style_notes[style_section] + 24;							
						midi_send_note(0x90, note, 127);
						old_midinotes[0] = note;		
					}						
				} 
				else 
				{
					while (strum_pattern[active_strum_pattern][seq_index][0] == 0 ) {		// ignore empty pattern steps	
						seq_index++;
						if (seq_index > 11) seq_index = 0;
					}

					for (int i=0; i<6; i++) {
						mute_midinotes[i] = 0;	// reset muted notes
						string = 6 - strum_pattern[active_strum_pattern][seq_index][i];
						
						if (string > -1 && string < 6) 
						{
							if (chord_chat[chord_note % 12][chord_type][string] > -1) {	// ignore unused strings
								chord_midinotes[notes_count] = string_frets[string] + chord_chat[chord_note % 12][chord_type][string];
								notes_count++;						
							}
						}
					}

					velocity = 127;
					
					if (up) {
						qsort(chord_midinotes, notes_count, sizeof(uint8_t), compUp);			
					} else {
						qsort(chord_midinotes, notes_count, sizeof(uint8_t), compDown);								
					}
				
					for (int n=0; n<notes_count; n++) {
						note = chord_midinotes[(notes_count - 1) - n];											
						velocity = velocity - 15;
						
						if (active_neck_pos == 1) {
							if ((note % 12) > 6) note = note - 12; 	// bass needs another octave lower for bass neck pos
						}						
						
						if (enable_ample_guitar && note < 40) note = note + 12;	
						
						midi_send_note(0x90, note, velocity);
						old_midinotes[n] = note;
						mute_midinotes[n] = note;							
					}	

					seq_index++;	
					if (seq_index > 11) seq_index = 0;
				}						
				
			} else {
				note = ((bass_note ? bass_note : chord_note) % 12) + (O * (active_neck_pos + 2));
				
				if (active_strum_pattern == 1 || active_neck_pos == 1) {
					if ((note % 12) > 6) note = note - 12; 	// bass needs another octave lower for strum pattern 2 or bass
				}
				
				if (enable_ample_guitar && note < 40) note = note + 12;				 
				midi_send_note(0x90, note, velocity);
				old_midinotes[0] = note;				
			}					
		} 
		else 
		{
			if (enable_ample_guitar) 	// noises
			{
				if (active_neck_pos == 1) {	// bass slides
					note = 95;
					if (up) note = 94;
				}					
				else
					
				if (active_strum_pattern == 0) {
					note = 93;
					if (up) note = 94;
				}
				else

				if (active_strum_pattern == 1) {
					note = 95;
					if (up) note = 94;
				}					
				else

				if (active_strum_pattern == 2) {
					note = 89;
					if (up) note = 92;
				}				
				else

				if (active_strum_pattern == 3) {
					note = 89;
					if (up) note = 91;
				}					
				else

				if (active_strum_pattern == 4) {
					note = 89;
					if (up) note = 90;
				}
				
				midi_send_note(0x90, note, 100);				
				old_midinotes[0] = note;
			} 
			else 
			{	
				if (enable_midi_drums && looper_status.state == LOOPER_STATE_RECORDING) 
				{					
					if (style_section % 2 == 0) {
						looper_status.current_track = 0;					// Bass Drum
						if (up) looper_status.current_track = 1;			// Snare
						looper_handle_input_internal_clock(BUTTON_EVENT_CLICK_BEGIN);					}
					else
						
					if (style_section % 2 == 1) {
						looper_status.current_track = 2;					// Closed Hit-Hat
						if (up) looper_status.current_track = 3;			// Open Hit-Hat
						looper_handle_input_internal_clock(BUTTON_EVENT_CLICK_BEGIN);						
					}										
				}
				else
					
				if (active_strum_pattern == 0) 	
				{					
					for (int n=0; n<6; n++) // only mute played notes with strumming up and down
					{
						if (mute_midinotes[n] > 0) {
							note = mute_midinotes[n];
							midi_send_note(0x90, note, 35);	// lower velocity to achieve muted sound
							old_midinotes[n] = note;					
						}
					}					
				}
			}
		}
	} 		
}

static void pico_bluetooth_on_oob_event(uni_platform_oob_event_t event, void* data) {
  switch (event) {
    case UNI_PLATFORM_OOB_GAMEPAD_SYSTEM_BUTTON:
      // Optional: do something when "system" button gets pressed.
	  //logi("my_platform_on_oob_event: system button event: 0x%04x\n", event);
      break;

    case UNI_PLATFORM_OOB_BLUETOOTH_ENABLED:
      // When the "bt scanning" is on / off. Could be triggered by different events
      // Useful to notify the user
      // logi("my_platform_on_oob_event: Bluetooth enabled: %d\n", (bool)(data));
      break;

    default:
      //logi("my_platform_on_oob_event: unsupported event: 0x%04x\n", event);
	  break;
  }
}

struct uni_platform* get_my_platform(void) {
  static struct uni_platform plat = {
      .name = "Pico2 W",
      .init = pico_bluetooth_init,
      .on_init_complete = pico_bluetooth_on_init_complete,
      .on_device_discovered = pico_bluetooth_on_device_discovered,
      .on_device_connected = pico_bluetooth_on_device_connected,
      .on_device_disconnected = pico_bluetooth_on_device_disconnected,
      .on_device_ready = pico_bluetooth_on_device_ready,
      .on_oob_event = pico_bluetooth_on_oob_event,
      .on_controller_data = pico_bluetooth_on_controller_data,
      .get_property = pico_bluetooth_get_property,
  };

  return &plat;
}

void bluetooth_init(void) {
  //PICO_DEBUG("[INIT] Starting Bluetooth initialization...\n");

  // Keep Wi-Fi off but don't fully disable to avoid interfering with Bluetooth
  // cyw43_arch_disable_sta_mode();
  cyw43_arch_disable_ap_mode();

  // Must be called before uni_init()
  uni_platform_set_custom(get_my_platform());
  // PICO_DEBUG("[INIT] Custom platform registered\n");

  // Initialize BP32
  uni_init(0, NULL);
  // PICO_INFO("Bluepad32 initialized\n");
}