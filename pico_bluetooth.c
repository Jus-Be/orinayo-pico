#include "pico_bluetooth.h"

#include <stddef.h>
#include <string.h>

#include <bt/uni_bt.h>
#include <btstack.h>
#include <controller/uni_gamepad.h>
#include "pico/stdlib.h"
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
bool strum_neutral = true;
bool style_started = false;
bool enable_style_play = false;
bool enable_auto_hold = false;
bool enable_seqtrak = false;
bool enable_arranger_mode = false;
bool enable_modx = false;
bool enable_chord_track = true;
bool enable_bass_track = true;
bool enable_ample_guitar = false;
bool enable_midi_drums = false;
bool gamepad_guitar_connected = false;

uint8_t but0;
uint8_t but1;
uint8_t but2;
uint8_t but3;
uint8_t but4; 
uint8_t but6;
uint8_t but7;   
uint8_t but9;

uint8_t mbut0;
uint8_t mbut1;
uint8_t mbut2;
uint8_t mbut3;

uint8_t dpad_left;	
uint8_t dpad_right;
uint8_t dpad_up;
uint8_t dpad_down;

bool joy_up;  
bool joy_down;  
bool knob_up; 
bool knob_down; 

uint8_t green = 0;
uint8_t red = 0;
uint8_t yellow = 0;
uint8_t blue = 0;
uint8_t orange = 0;
uint8_t starpower = 0;
uint8_t pitch = 0;
uint8_t song_key = 0;

uint8_t up = 0;
uint8_t down = 0;
uint8_t left = 0;
uint8_t right = 0;	

uint8_t start = 0;
uint8_t menu = 0;
uint8_t logo = 0;
uint8_t config = 0;	

uint8_t joystick_up = 0;
uint8_t joystick_down = 0;  
uint8_t logo_knob_up = 0;  
uint8_t logo_knob_down = 0; 	

int applied_velocity = 100;
int guitar_pc_code = 26;
int active_strum_pattern = 0;	
int active_neck_pos = 2;
int style_section = 0; 
int style_group = 0; 
int old_style = -1;
int ample_old_key = 0;
int last_chord_note = 0;
int last_chord_type = 0;
int seqtrak_chord = 0;
int transpose = 0; 
int midi_current_step = 0;

uint8_t voice_note = 0;
uint8_t chord_notes[6] = {0};
uint8_t old_midinotes[6] = {0};
uint8_t mute_midinotes[6] = {0};

void midi_modx_arp_octave(uint8_t octave);
void midi_modx_arp(bool on);
void midi_modx_arp_hold(uint8_t part, bool on);
void midi_modx_arp_realtime(uint8_t part, bool on);
void midi_modx_tempo(int tempo);
void midi_modx_key(uint8_t key);
void midi_seqtrak_arp();
void midi_seqtrak_key(uint8_t key);
void midi_seqtrak_tempo(int tempo);
void midi_seqtrak_pattern(uint8_t pattern);
void midi_seqtrak_mute(uint8_t track, bool mute);
void midi_start_stop(bool start);
void midi_send_note(uint8_t command, uint8_t note, uint8_t velocity);
void midi_send_program_change(uint8_t command, uint8_t code);
void midi_send_control_change(uint8_t command, uint8_t controller, uint8_t value);
void midi_play_chord(bool on, uint8_t p1, uint8_t p2, uint8_t p3);
void midi_play_slash_chord(bool on, uint8_t p1, uint8_t p2, uint8_t p3, uint8_t p4);
void midi_ketron_arr(uint8_t code, bool on);
void midi_ketron_footsw(uint8_t code, bool on);
void midi_yamaha_start_stop(uint8_t code, bool on);
void midi_yamaha_arr(uint8_t code, bool on);
void midi_process_state(uint64_t start_us);
void midi_bluetooth_handle_data();

void play_chord(bool on, bool up, uint8_t green, uint8_t red, uint8_t yellow, uint8_t blue, uint8_t orange);
void clear_chord_notes();
void stop_chord();

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

uint8_t strum_styles[5][5][16][3] = {
	{
		{{74, 77, 114}, {77, 74, 24}, {78, 77, 90}, {77, 78, 90}, {74, 77, 121}, {77, 74, 91}, {77, 77, 90}, {76, 77, 107}, {77, 76, 82}, {76, 77, 121}, {78, 76, 90}, {77, 78, 90}, {76, 77, 123}, {77, 76, 80}, {78, 77, 90}, {77, 78, 90}},		
		{{64, 83, 91},  {0, 0, 0},    {64, 64, 50}, {0, 0, 0},    {74, 64, 103}, {0, 0, 0},    {79, 74, 71}, {76, 79, 90},  {77, 76, 64}, {76, 77, 89},  {77, 76, 67}, {83, 77, 71}, {74, 83, 101}, {0, 0, 0},    {79, 74, 64}, {83, 79, 40}},	
		{{64, 81, 88},  {0, 0, 0},    {0, 64, 0},   {0, 0, 0},    {78, 0, 26},   {0, 78, 0},   {64, 0, 60},  {0, 0, 0},     {0, 0, 0},    {0, 0, 0},     {64, 0, 90},  {0, 64, 0},   {81, 0, 75},   {0, 0, 0},    {0, 0, 0},    {0, 0, 0}},	
		{{72, 77, 126}, {78, 72, 1},  {78, 78, 58}, {76, 78, 100},{78, 76, 1},   {0, 78, 0},   {72, 0, 123}, {0, 0, 0},     {0, 0, 0},    {0, 0, 0},     {64, 72, 90}, {0, 64, 0},   {72, 0, 109},  {78, 72, 90}, {77, 78, 90}, {77, 77, 64}},	
		{{72, 76, 89},  {0, 0, 0},    {76, 72, 55}, {0, 0, 0},    {77, 76, 100}, {0, 0, 0},    {76, 77, 70}, {0, 0, 0},     {74, 76, 72}, {0, 74, 0},    {76, 0, 65},  {0, 0, 0},    {77, 76, 60},   {0, 0, 0},    {76, 77, 91}, {0, 0, 0}}
	},
	{
		{{0, 0, 0},     {0, 0, 0},    {0, 0, 0},    {0, 0, 0},    {0, 0, 0},     {0, 0, 0},    {0, 0, 0},    {0, 0, 0},     {0, 0, 0},    {0, 0, 0},     {0, 0, 0},    {0, 0, 0},    {0, 0, 0},     {0, 0, 0},    {0, 0, 0},    {0, 0, 0}},	
		{{0, 0, 0},     {0, 0, 0},    {0, 0, 0},    {0, 0, 0},    {0, 0, 0},     {0, 0, 0},    {0, 0, 0},    {0, 0, 0},     {0, 0, 0},    {0, 0, 0},     {0, 0, 0},    {0, 0, 0},    {0, 0, 0},     {0, 0, 0},    {0, 0, 0},    {0, 0, 0}},	
		{{0, 0, 0},     {0, 0, 0},    {0, 0, 0},    {0, 0, 0},    {0, 0, 0},     {0, 0, 0},    {0, 0, 0},    {0, 0, 0},     {0, 0, 0},    {0, 0, 0},     {0, 0, 0},    {0, 0, 0},    {0, 0, 0},     {0, 0, 0},    {0, 0, 0},    {0, 0, 0}},	
		{{0, 0, 0},     {0, 0, 0},    {0, 0, 0},    {0, 0, 0},    {0, 0, 0},     {0, 0, 0},    {0, 0, 0},    {0, 0, 0},     {0, 0, 0},    {0, 0, 0},     {0, 0, 0},    {0, 0, 0},    {0, 0, 0},     {0, 0, 0},    {0, 0, 0},    {0, 0, 0}},	
		{{0, 0, 0},     {0, 0, 0},    {0, 0, 0},    {0, 0, 0},    {0, 0, 0},     {0, 0, 0},    {0, 0, 0},    {0, 0, 0},     {0, 0, 0},    {0, 0, 0},     {0, 0, 0},    {0, 0, 0},    {0, 0, 0},     {0, 0, 0},    {0, 0, 0},    {0, 0, 0}}	
	},
	{
		{{0, 0, 0},     {0, 0, 0},    {0, 0, 0},    {0, 0, 0},    {0, 0, 0},     {0, 0, 0},    {0, 0, 0},    {0, 0, 0},     {0, 0, 0},    {0, 0, 0},     {0, 0, 0},    {0, 0, 0},    {0, 0, 0},     {0, 0, 0},    {0, 0, 0},    {0, 0, 0}},	
		{{0, 0, 0},     {0, 0, 0},    {0, 0, 0},    {0, 0, 0},    {0, 0, 0},     {0, 0, 0},    {0, 0, 0},    {0, 0, 0},     {0, 0, 0},    {0, 0, 0},     {0, 0, 0},    {0, 0, 0},    {0, 0, 0},     {0, 0, 0},    {0, 0, 0},    {0, 0, 0}},	
		{{0, 0, 0},     {0, 0, 0},    {0, 0, 0},    {0, 0, 0},    {0, 0, 0},     {0, 0, 0},    {0, 0, 0},    {0, 0, 0},     {0, 0, 0},    {0, 0, 0},     {0, 0, 0},    {0, 0, 0},    {0, 0, 0},     {0, 0, 0},    {0, 0, 0},    {0, 0, 0}},	
		{{0, 0, 0},     {0, 0, 0},    {0, 0, 0},    {0, 0, 0},    {0, 0, 0},     {0, 0, 0},    {0, 0, 0},    {0, 0, 0},     {0, 0, 0},    {0, 0, 0},     {0, 0, 0},    {0, 0, 0},    {0, 0, 0},     {0, 0, 0},    {0, 0, 0},    {0, 0, 0}},	
		{{0, 0, 0},     {0, 0, 0},    {0, 0, 0},    {0, 0, 0},    {0, 0, 0},     {0, 0, 0},    {0, 0, 0},    {0, 0, 0},     {0, 0, 0},    {0, 0, 0},     {0, 0, 0},    {0, 0, 0},    {0, 0, 0},     {0, 0, 0},    {0, 0, 0},    {0, 0, 0}}	
	},
	{
		{{0, 0, 0},     {0, 0, 0},    {0, 0, 0},    {0, 0, 0},    {0, 0, 0},     {0, 0, 0},    {0, 0, 0},    {0, 0, 0},     {0, 0, 0},    {0, 0, 0},     {0, 0, 0},    {0, 0, 0},    {0, 0, 0},     {0, 0, 0},    {0, 0, 0},    {0, 0, 0}},	
		{{0, 0, 0},     {0, 0, 0},    {0, 0, 0},    {0, 0, 0},    {0, 0, 0},     {0, 0, 0},    {0, 0, 0},    {0, 0, 0},     {0, 0, 0},    {0, 0, 0},     {0, 0, 0},    {0, 0, 0},    {0, 0, 0},     {0, 0, 0},    {0, 0, 0},    {0, 0, 0}},	
		{{0, 0, 0},     {0, 0, 0},    {0, 0, 0},    {0, 0, 0},    {0, 0, 0},     {0, 0, 0},    {0, 0, 0},    {0, 0, 0},     {0, 0, 0},    {0, 0, 0},     {0, 0, 0},    {0, 0, 0},    {0, 0, 0},     {0, 0, 0},    {0, 0, 0},    {0, 0, 0}},	
		{{0, 0, 0},     {0, 0, 0},    {0, 0, 0},    {0, 0, 0},    {0, 0, 0},     {0, 0, 0},    {0, 0, 0},    {0, 0, 0},     {0, 0, 0},    {0, 0, 0},     {0, 0, 0},    {0, 0, 0},    {0, 0, 0},     {0, 0, 0},    {0, 0, 0},    {0, 0, 0}},	
		{{0, 0, 0},     {0, 0, 0},    {0, 0, 0},    {0, 0, 0},    {0, 0, 0},     {0, 0, 0},    {0, 0, 0},    {0, 0, 0},     {0, 0, 0},    {0, 0, 0},     {0, 0, 0},    {0, 0, 0},    {0, 0, 0},     {0, 0, 0},    {0, 0, 0},    {0, 0, 0}}	
	},
	{
		{{0, 0, 0},     {0, 0, 0},    {0, 0, 0},    {0, 0, 0},    {0, 0, 0},     {0, 0, 0},    {0, 0, 0},    {0, 0, 0},     {0, 0, 0},    {0, 0, 0},     {0, 0, 0},    {0, 0, 0},    {0, 0, 0},     {0, 0, 0},    {0, 0, 0},    {0, 0, 0}},	
		{{0, 0, 0},     {0, 0, 0},    {0, 0, 0},    {0, 0, 0},    {0, 0, 0},     {0, 0, 0},    {0, 0, 0},    {0, 0, 0},     {0, 0, 0},    {0, 0, 0},     {0, 0, 0},    {0, 0, 0},    {0, 0, 0},     {0, 0, 0},    {0, 0, 0},    {0, 0, 0}},	
		{{0, 0, 0},     {0, 0, 0},    {0, 0, 0},    {0, 0, 0},    {0, 0, 0},     {0, 0, 0},    {0, 0, 0},    {0, 0, 0},     {0, 0, 0},    {0, 0, 0},     {0, 0, 0},    {0, 0, 0},    {0, 0, 0},     {0, 0, 0},    {0, 0, 0},    {0, 0, 0}},	
		{{0, 0, 0},     {0, 0, 0},    {0, 0, 0},    {0, 0, 0},    {0, 0, 0},     {0, 0, 0},    {0, 0, 0},    {0, 0, 0},     {0, 0, 0},    {0, 0, 0},     {0, 0, 0},    {0, 0, 0},    {0, 0, 0},     {0, 0, 0},    {0, 0, 0},    {0, 0, 0}},	
		{{0, 0, 0},     {0, 0, 0},    {0, 0, 0},    {0, 0, 0},    {0, 0, 0},     {0, 0, 0},    {0, 0, 0},    {0, 0, 0},     {0, 0, 0},    {0, 0, 0},     {0, 0, 0},    {0, 0, 0},    {0, 0, 0},     {0, 0, 0},    {0, 0, 0},    {0, 0, 0}}	
	}	
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
  gamepad_guitar_connected = false;
  uni_bt_start_scanning_and_autoconnect_safe();
  
  cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, true);  
  // PICO_DEBUG("[BT] Restarted scanning (device disconnected)\n");
}

static uni_error_t pico_bluetooth_on_device_ready(uni_hid_device_t* d) {
  // You can reject the connection by returning an error.
  cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, false);  	
  gamepad_guitar_connected = true;
  return UNI_ERROR_SUCCESS;
}

static const uni_property_t* pico_bluetooth_get_property(uni_property_idx_t idx) {
  ARG_UNUSED(idx);
  return NULL;
}

static void pico_bluetooth_on_controller_data(uni_hid_device_t* d, uni_controller_t* ctl) { 
	uint8_t axis_x = ctl->gamepad.axis_x / 4;
	uint8_t axis_y = ctl->gamepad.axis_y / 4;
	uint8_t axis_rx = ctl->gamepad.axis_rx / 4;
	uint8_t axis_ry = ctl->gamepad.axis_ry / 4;
	
	joy_up = axis_y > axis_x;  
	joy_down = axis_x > axis_y;  
	knob_up = axis_ry > axis_rx; 
	knob_down = axis_rx > axis_ry; 	
  
	but0 = (ctl->gamepad.buttons >> 0) & 0x01;
	but1 = (ctl->gamepad.buttons >> 1) & 0x01;
	but2 = (ctl->gamepad.buttons >> 2) & 0x01;
	but3 = (ctl->gamepad.buttons >> 3) & 0x01;
	but4 = (ctl->gamepad.buttons >> 4) & 0x01; 
	but6 = (ctl->gamepad.buttons >> 6) & 0x01;   
	but7 = (ctl->gamepad.buttons >> 7) & 0x01;   
	but9 = (ctl->gamepad.buttons >> 9) & 0x01;	
	
	dpad_left = ctl->gamepad.dpad & 0x02;	
	dpad_right = ctl->gamepad.dpad & 0x01;
	dpad_up = ctl->gamepad.dpad & 0x04;
	dpad_down = ctl->gamepad.dpad & 0x08;	
	
	mbut0 = (ctl->gamepad.misc_buttons >> 0) & 0x01;
	mbut1 = (ctl->gamepad.misc_buttons >> 1) & 0x01;
	mbut2 = (ctl->gamepad.misc_buttons >> 2) & 0x01;
	mbut3 = (ctl->gamepad.misc_buttons >> 3) & 0x01;

	switch (ctl->klass) {
		case UNI_CONTROLLER_CLASS_GAMEPAD:		
			midi_bluetooth_handle_data(); 
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

void midi_bluetooth_handle_data() {
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
			
		if (red && yellow && blue) 
		{
			if (but6) {
				enable_auto_hold = !enable_auto_hold;
				midi_modx_arp_hold(0, enable_auto_hold);	// only control part 1
			}					
		}
		else

		if (green && yellow) 
		{
			if (but6) {
				enable_chord_track = !enable_chord_track;
			}
		}
		else				
			
		if (red && blue) 	
		{				
			if (but6) {
				enable_bass_track = !enable_bass_track;									
			}
		}
		else

		if (yellow && orange) {
			if (but6) enable_style_play = !enable_style_play;	// toggle chord generation
		}				
		else

		if (yellow && blue) {
			active_neck_pos = 3;	// High
			
			if (but6) 
			{
				if (enable_arranger_mode) 	{
					midi_send_program_change(0xC0, guitar_pc_code);	// electric jazz guitar on channel 1						
				}
				else
					
				if (enable_seqtrak) 		midi_seqtrak_arp();	
				else						
				if (enable_modx) 			midi_modx_arp_octave(active_neck_pos - 2);	// use neck position to set keyboard octave					
			}				
		}
		else

		if (yellow && red) {
			active_neck_pos = 2;	// Normal
			
			if (but6) 
			{
				if (enable_arranger_mode) 	{
					midi_send_program_change(0xC0, guitar_pc_code);	// electric jazz guitar on channel 1						
				}
				else
					
				if (enable_seqtrak) 		midi_seqtrak_arp();	
				else						
				if (enable_modx) 			midi_modx_arp_octave(active_neck_pos - 2);	// use neck position to set keyboard octave					
			}				
		}
		else
			
		if (green && red) {
			active_neck_pos = 1;	// Low
			
			if (but6) 
			{
				if (enable_arranger_mode) 	{
					midi_send_program_change(0xC0, 33);	// electric bass guitar on channel 1						
					active_strum_pattern = 3;			// force arpeggios, no strumming
				}
				else
					
				if (enable_seqtrak) 		midi_seqtrak_arp();	
				else						
				if (enable_modx) 			midi_modx_arp_octave(active_neck_pos - 2);	// use neck position to set keyboard octave	
			}
		}
		else
			
		if (blue && orange) {
			active_strum_pattern = -1;
			if (but6 && enable_seqtrak) midi_seqtrak_arp();
		}
		else			
						
		if (green) {
			active_strum_pattern = 0;
			if (but6 && enable_seqtrak) midi_seqtrak_arp();				
			
			if (but6 && enable_ample_guitar && active_neck_pos != 1) {
				midi_send_note(0x90, 86, 127);		// enable chord detection					
				midi_send_note(0x90, 97, 127);		// key switch for strum mode on
				midi_send_note(0xB0, 64, 1);		// hold pedal off	
			}
		}
		else
			
		if (yellow) {
			active_strum_pattern = 2;
			if (but6 && enable_seqtrak) midi_seqtrak_arp();
			
			if (but6 && enable_ample_guitar && active_neck_pos != 1) {
				midi_send_note(0x90, 97, 1);		// key switch for strum mode off
				midi_send_note(0xB0, 64, 127);		// hold pedal on	
				midi_send_note(0x90, 99, 127);					
			}
		}
		else

		if (blue) {
			active_strum_pattern = 3;
			if (but6 && enable_seqtrak) midi_seqtrak_arp();
			
			if (but6 && enable_ample_guitar && active_neck_pos != 1) {
				midi_send_note(0x90, 97, 1);		// key switch for strum mode off
				midi_send_note(0xB0, 64, 127);		// hold pedal on	
				midi_send_note(0x90, 99, 127);						
			}
		}				
		else

		if (red) {
			active_strum_pattern = 1;
			if (but6 && enable_seqtrak) midi_seqtrak_arp();
			
			if (but6 && enable_ample_guitar && active_neck_pos != 1) {
				midi_send_note(0x90, 97, 1);		// key switch for strum mode off
				midi_send_note(0xB0, 64, 1);		// hold pedal off						
			}				
		}
		else

		if (orange) {
			active_strum_pattern = 4;
			if (but6 && enable_seqtrak) midi_seqtrak_arp();
			
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
			}

			if (enable_midi_drums)	
			{
				if (but6 && looper_status.state == LOOPER_STATE_PLAYING) {
					//ghost_parameters_t *params = ghost_note_parameters();
					//params->ghost_intensity = 0.843;	
					storage_store_tracks();						
				}							
			}	
			else
				
			if (enable_seqtrak) 
			{
				if (but6) {
					midi_seqtrak_arp();
					midi_seqtrak_pattern(style_section % 6);				
				}
			}
			else
				
			if (enable_modx) 
			{
				if (but6) {
					uint8_t modx_scenes[8] = {0, 16, 32, 48, 64, 80, 96, 112};
					midi_send_control_change(0xB3, 92, modx_scenes[style_section % 8]);
				}
			}
			else

			if (enable_arranger_mode) {
				midi_ketron_arr(3 + (style_section % 4), but6 ? true : false);
				midi_yamaha_arr(0x10 + (style_section % 4), but6 ? true : false);						
			}
		}	
		
		if (but6) {
			if (green) midi_send_control_change(0xB3, 9, 1); 		// Melody voice -1
			else if (red) midi_send_control_change(0xB3, 9, 2); 	// Melody voice -2					
			else if (yellow) midi_send_control_change(0xB3, 9, 3); 	// Melody voice -3						
			else if (blue) midi_send_control_change(0xB3, 9, 4); 	// Melody voice -4	
			else if (orange) midi_send_control_change(0xB3, 9, 5); 	// Melody voice -5
			else midi_send_control_change(0xB3, 14, 127); 			// Previous Style
		}			

		return;
	}

	// handle direct key change (D, E, F, G, A)

	if (but7 != song_key)  {	
		song_key = but7;

		if (but7) {
			transpose = 0;
			
			if (green) 	transpose = 2;		// D
			if (red) 	transpose = 4;		// E
			if (yellow)	transpose = 5;		// F
			if (blue) 	transpose = 7;		// G				
			if (orange) transpose = 9;		// A
		}
		return;			
	}

	// handle actions

	if (but9 != start)  {	// unused because start button clashes with axis (knob_up/knob_down)
		start = but9;		
		return;			
	}				

	if (dpad_left != left) { 	// Strum down
		left = dpad_left;
		
		if (dpad_left) 	{
			strum_neutral = false;
			if (!enable_auto_hold) stop_chord();			
			play_chord(true, false, green, red, yellow, blue, orange);
			
		} else {
			strum_neutral = true;
			
			if ((!green && !red && !yellow && !blue && !orange) || active_strum_pattern == 0 || active_strum_pattern == 1 || enable_auto_hold) {
				stop_chord();	// sustain arpeggios only
			}
			
			if (looper_status.state == LOOPER_STATE_RECORDING || looper_status.state == LOOPER_STATE_TAP_TEMPO) {
				looper_handle_input_internal_clock(BUTTON_EVENT_CLICK_RELEASE);				
			}
		}
		
		if (!style_started) cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, !!dpad_left);			
		return;
	}		

	if (dpad_right != right) {	// strum up
		right = dpad_right;	

		if (dpad_right) {
			strum_neutral = false;				
			if (!enable_auto_hold) stop_chord();
			play_chord(true, true, green, red, yellow, blue, orange);
			
		} else {
			strum_neutral = true;
			
			if ((!green && !red && !yellow && !blue && !orange) || active_strum_pattern == 0 || active_strum_pattern == 1 || enable_auto_hold) {
				stop_chord();	// sustain arpeggios only
			}	

			if (looper_status.state == LOOPER_STATE_RECORDING || looper_status.state == LOOPER_STATE_TAP_TEMPO) {
				looper_handle_input_internal_clock(BUTTON_EVENT_CLICK_RELEASE);				
			}						
		}

		if (!style_started) cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, !!dpad_right);				
		return;
	}

	if (dpad_up != up) {	// transpose down
		up = dpad_up;

		if (dpad_up) {
			transpose--;
			if (transpose < 0) 	transpose = 11;				
			if (enable_seqtrak) midi_seqtrak_key(transpose);				
			//if (enable_modx) 	midi_modx_key(transpose);				
		}
		return;			
	}

	if (dpad_down != down) {	// transpose up
		down = dpad_down;
		
		if (dpad_down) {
			transpose++;
			if (transpose > 11) transpose = 0;	
			if (enable_seqtrak) midi_seqtrak_key(transpose);
			//if (enable_modx) 	midi_modx_key(transpose);				
		}
		return;
	}		

	uint8_t ketron_code;
	uint8_t yamaha_code;

	if (mbut0 != logo) {
		logo = mbut0;
		
		if (enable_midi_drums) 
		{
			if (mbut0) {
				
				if (looper_status.state == LOOPER_STATE_WAITING || looper_status.state == LOOPER_STATE_RECORDING || looper_status.state == LOOPER_STATE_TAP_TEMPO) {
					style_section = 0;	
					looper_status.current_step = 0;	
					
					if (looper_status.state == LOOPER_STATE_RECORDING) storage_store_tracks();													
					looper_status.state = LOOPER_STATE_PLAYING;
					
					//ghost_parameters_t *params = ghost_note_parameters();						
					//params->ghost_intensity = 0.843;							
				} 
				else 
				
				if (looper_status.state == LOOPER_STATE_PLAYING) {
					looper_status.state = LOOPER_STATE_WAITING;
				}	
			}					
		} 
		else {
		
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
			
			if (enable_arranger_mode) midi_ketron_arr(ketron_code, mbut0 ? true : false);
			
			if (!style_started) {
				if (yamaha_code != 127) {
					if (enable_arranger_mode) midi_yamaha_arr(yamaha_code, mbut0 ? true : false);	
				} 
				
				if (mbut0) {						
					if (enable_seqtrak) {
						midi_seqtrak_mute(7, false);
						midi_seqtrak_mute(9, false);
						
						midi_start_stop(true);							
					} 
					else
						
					if (enable_modx) {					
						midi_modx_arp(true);
					}
					else 
					
					if (enable_arranger_mode) {
						midi_yamaha_start_stop(0x7A, true);							
					}
					else {
						if (green) midi_send_control_change(0xB3, 3, 1); 		// Fill-1
						else if (red) midi_send_control_change(0xB3, 3, 2); 	// Fill-2						
						else if (yellow) midi_send_control_change(0xB3, 3, 3); 	// Fill-3						
						else if (blue) midi_send_control_change(0xB3, 3, 4); 	// Sync start	
						else if (orange) midi_send_control_change(0xB3, 3, 5); 	// Fade In
						else midi_send_control_change(0xB3, 3, 65); 			// Play
					}
				}
				
			} else {
				if (yamaha_code != 127) {
					if (enable_arranger_mode) midi_yamaha_arr(0x20 + yamaha_code, mbut0 ? true : false);	
				}
				
				if (mbut0) {
					if (enable_seqtrak) {	
						midi_seqtrak_mute(7, true);
						midi_seqtrak_mute(9, true);	
						
						midi_start_stop(false);
						midi_play_chord(false, 0, 0, 0);		
					} 
					else
						
					if (enable_modx) {
						midi_modx_arp(false);				
					}						
					else 
					
					if (enable_arranger_mode) {
						midi_yamaha_start_stop(0x7D, true);		
					}	
					else {
						if (yellow) midi_send_control_change(0xB3, 3, 66); 		// End-1
						else if (red) midi_send_control_change(0xB3, 3, 67); 	// End-2						
						else if (green) midi_send_control_change(0xB3, 3, 68); 	// End-3						
						else if (blue) midi_send_control_change(0xB3, 3, 69); 	// Sync stop	
						else if (orange) midi_send_control_change(0xB3, 3, 70); // Fade Out
						else midi_send_control_change(0xB3, 3, 127); 			// Stop
					}						
				}						
			}
		}
		
		if (mbut0) style_started = !style_started;
		cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, style_started);			
		return;
	}		

	if (mbut1 != starpower) { // next style/section	
		bool style_selected = false;
		starpower = mbut1;			
		if (mbut1) old_style = style_section;

		if (green) 
		{
			if (mbut1) {
				style_selected = true;
				style_section = 0;
			}
		}
		else
			
		if (red) 
		{
			if (mbut1) {
				style_selected = true;
				style_section = 1;
			}
		}
		else

		if (yellow) 
		{
			if (mbut1) {
				style_selected = true;
				style_section = 2;
			}
		}				
		else

		if (blue) 
		{
			if (mbut1) {
				style_selected = true;
				style_section = 3;
			}
		}
		else

		if (orange) 
		{
			if (mbut1) {
				style_selected = true;
				style_section = 7;
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
				//ghost_parameters_t *params = ghost_note_parameters();
				//params->ghost_intensity = 0.843;	
				storage_store_tracks();						
			}				
		}
		else
			
		if (enable_seqtrak) 
		{
			if (mbut1) 
			{
				if (style_started) {
					midi_seqtrak_arp();
					midi_seqtrak_pattern(style_section % 6);				
				} 
			}
		} 
		else
			
		if (enable_modx) 
		{
			if (mbut1) {
				uint8_t modx_scenes[8] = {0, 16, 32, 48, 64, 80, 96, 112};
				midi_send_control_change(0xB3, 92, modx_scenes[style_section % 8]);
			}
		}			
		else 
		
		if (enable_arranger_mode) {	
			midi_ketron_arr(3 + (style_section % 4), mbut1 ? true : false);
			midi_yamaha_arr(0x10 + (style_section % 4), mbut1 ? true : false);	
		}	

		if (mbut1) {
			if (green) midi_send_control_change(0xB3, 14, 1); 		// Style select -1
			else if (red) midi_send_control_change(0xB3, 14, 2); 	// Style select -2					
			else if (yellow) midi_send_control_change(0xB3, 14, 3); // Style select -3						
			else if (blue) midi_send_control_change(0xB3, 14, 4); 	// Style select -4	
			else if (orange) midi_send_control_change(0xB3, 14, 5); // Style select -5
			else midi_send_control_change(0xB3, 14, 65); 			// Next Style
		}			
		
		return;			
	}

	if (mbut2 != menu) {
		if (enable_arranger_mode) midi_ketron_footsw(8, mbut2 ? true : false);						// 	user defined from footswitch	
		menu = mbut2;

		if (green && yellow) 
		{
			if (mbut2) {
				style_group = 12;	
			}
		}
		else
			
		if (red && blue) 
		{
			if (mbut2) {
				style_group = 11;	
			}
		}
		else

		if (yellow && orange) 
		{
			if (mbut2) {
				style_group = 10;
			}
		}				
		else
			
		if (green && yellow) 
		{
			if (mbut2) {
				style_group = 9;	
			}
		}
		else
			
		if (green && red) 
		{
			if (mbut2) {
				style_group = 8;	
			}
		}
		else

		if (red && yellow) 
		{
			if (mbut2) {
				style_group = 7;
			}
		}				
		else

		if (yellow && blue) 
		{
			if (mbut2) {
				style_group = 6;	
			}
		}
		else

		if (blue && orange) 
		{
			if (mbut2) {
				style_group = 5;
			}
		}
		else
			
		if (green) 
		{
			if (mbut2) {
				style_group = 0;	
			}
		}
		else
			
		if (red) 
		{
			if (mbut2) {
				style_group = 1;	
			}
		}
		else

		if (yellow) 
		{
			if (mbut2) {
				style_group = 2;
			}
		}				
		else

		if (blue) 
		{
			if (mbut2) {
				style_group = 3;	
			}
		}
		else

		if (orange) 
		{
			if (mbut2) {
				style_group = 4;
			}
		}

		if (enable_seqtrak) 
		{
			if (mbut2) {
				midi_send_program_change(0xC0, style_group % 8);	// set PC to project no
				midi_send_control_change(0xB0, 0, 64); 				// MSB 64						
				midi_send_control_change(0xB0, 32, 0); 				// LSB 0
			}
		}
		else 
			
		if (enable_modx) 
		{
			if (mbut2) {
				// TODO - FIX!!!
				midi_send_program_change(0xC0, style_group % 16);	// set PC to performance/set list no						
				midi_send_control_change(0xB0, 0, 62); 				// MSB 62	
				midi_send_control_change(0xB0, 32, 0); 				// LSB 0 Page 1											
			}
		}
		else {
			if (mbut2) {
				midi_send_control_change(0xB3, 15, style_group + 1); 		// select style group
			}							
		}
			
			
		return;		
	}		

	if (mbut3 != config) {	// config/menu options
		config = mbut3;
		
		if (mbut3) 	{
			midi_play_chord(false, 0, 0, 0);	// reset chord  keys
			
			if (green && red) {
				guitar_pc_code = (guitar_pc_code == 26) ? 25 : 26;
				midi_send_program_change(0xC0, guitar_pc_code);	
			}
			else
			
			if (green) {  
				enable_arranger_mode = !enable_arranger_mode;
				enable_style_play = enable_arranger_mode;
			
				if (enable_arranger_mode) {				
					midi_send_program_change(0xC3, 89);		// warm pad on channel 4 (chords) 
					midi_send_control_change(0xB3, 7, 0); 	// don't play pads by default
					
					midi_send_program_change(0xC0, guitar_pc_code);		// jazz guitar on channel 1	
					midi_send_control_change(0xB0, 7, 100); // set default volume	
				}						
			}
			else
				
			if (red) {
				enable_ample_guitar = !enable_ample_guitar; 				// Ample Guitar VST mode
				enable_style_play = enable_ample_guitar;
				
				midi_send_note(0x90, 97, enable_ample_guitar ? 127 : 1);	// set strum mode on by default
				midi_send_note(0x90, 86, 127);
			}
			else
				
			if (yellow) {				
				if (enable_midi_drums) {
					looper_clear_all_tracks();								// Midi drums looper
				}
				enable_midi_drums = !enable_midi_drums;
			}
			else
				
			if (blue) {				
				enable_seqtrak = !enable_seqtrak;
				enable_style_play = enable_seqtrak;				
				
				if (enable_seqtrak) {					// initially mute seqtrak arpeggiator
					midi_seqtrak_mute(7, true);
					midi_seqtrak_mute(9, true);						
				}
			}
			else
				
			if (orange) {				
				enable_modx = !enable_modx;
				enable_style_play = enable_modx;					
				
				if (enable_modx) {						// set default scene 1
					midi_send_control_change(0xB3, 92, 0);						
				}
			}
		}
		
		return;			
	}

	if (joy_up != joystick_up) {
		joystick_up = joy_up;
		
		if (green) 
		{
			if (joy_up) {
				if (enable_midi_drums) 	looper_update_bpm(80);
				if (enable_seqtrak) 	midi_seqtrak_tempo(80);
				if (enable_modx) 		midi_modx_tempo(80);					
			}
		}
		else
			
		if (red) 
		{
			if (joy_up) {
				if (enable_midi_drums) 	looper_update_bpm(96);
				if (enable_seqtrak) 	midi_seqtrak_tempo(96);
				if (enable_modx) 		midi_modx_tempo(96);					
			}
		}
		else

		if (yellow) 
		{
			if (joy_up) {
				if (enable_midi_drums) 	looper_update_bpm(100);
				if (enable_seqtrak) 	midi_seqtrak_tempo(100);
				if (enable_modx) 		midi_modx_tempo(100);					
			}
		}
		else
			
		if (blue) 
		{
			if (joy_up) {
				if (enable_midi_drums) 	looper_update_bpm(110);
				if (enable_seqtrak) 	midi_seqtrak_tempo(110);
				if (enable_modx) 		midi_modx_tempo(110);					
			}
		}
		else	

		if (orange) 
		{
			if (joy_up) {
				if (enable_midi_drums) 	looper_update_bpm(120);
				if (enable_seqtrak) 	midi_seqtrak_tempo(120);
				if (enable_modx) 		midi_modx_tempo(120);					
			}
		}
		else					

		if (enable_ample_guitar) {
			midi_send_note(0x90, 24, joy_up ? 127 : 0);						// sustain guitar notes
		} 
		else
			
		if (enable_modx) 
		{
			if (joy_up && style_section > 0 && style_section < 4) {
				uint8_t modx_scenes[8] = {0, 16, 32, 48, 64, 80, 96, 112};
				
				midi_send_control_change(0xB3, 92, modx_scenes[style_section + 3]);
				sleep_ms(1000);				
				midi_send_control_change(0xB3, 92, modx_scenes[style_section]);					
			}
		}
		else

		if (enable_arranger_mode && style_started) {
			midi_ketron_arr(0x07 + (style_section % 4), joy_up ? true : false);	// 	Fill
			midi_yamaha_arr(0x10 + (style_section % 4), joy_up ? true : false);				
		}
		else {				
			if (joy_up) {
				midi_send_control_change(0xB3, 14, 6 + (style_section % 4)); 	// Fill
			}
		}
		return;
	}

	if (joy_down != joystick_down) {	// unused
		joystick_down = joy_down;					
		return;			
	}

	if (knob_up != logo_knob_up) {
		logo_knob_up = knob_up;	

		if (red) 
		{
			if (knob_up) 
			{
				if (enable_midi_drums)	{	
					ghost_parameters_t *params = ghost_note_parameters();
					params->ghost_intensity = 0.843;							
					return;
				}
			}
		}
		else
			
		if (yellow) 
		{
			if (knob_up) 
			{
				if (enable_midi_drums)	{	
					ghost_parameters_t *params = ghost_note_parameters();
					params->ghost_intensity = 0.0;					
					return;
				}
			}
		}
		else			
		
		if (blue) 
		{
			if (knob_up) 
			{
				if (!style_started && enable_midi_drums)	{						
					looper_status.state = LOOPER_STATE_RECORDING;
					
					if (style_group > -1) looper_clear_all_tracks();		// clear static style				
					style_group = -1;
					
					looper_status.current_step = 0;
					return;
				}
			}
		}
		else

		if (orange) 
		{
			if (knob_up) 
			{
				if (!style_started && enable_midi_drums)	{
					looper_status.state = LOOPER_STATE_TAP_TEMPO;	
					return;	
				}
			}
		}
		else

		if (enable_ample_guitar) {
			midi_send_note(0x90, 26, knob_up ? 127 : 0);	// palm mute guitar notes
		} 
		else
			
		if (enable_arranger_mode && style_started) {			
			midi_ketron_arr(0x0B + (style_section % 4), knob_up ? true : false);	// 	return	
			midi_yamaha_arr(0x18, knob_up ? true : false);				
		}
		else {				
			if (knob_up) {
				midi_send_control_change(0xB3, 14, 10); 	// Break
			}
		}
		
		return;			
	}

	if (knob_down != logo_knob_down) {	// unused
		logo_knob_down = knob_down;	

		if (red) 
		{
			if (knob_down) 
			{
				if (enable_midi_drums)	{	
					ghost_parameters_t *params = ghost_note_parameters();
					params->ghost_intensity = 0.843;							
					return;
				}
			}
		}
		else
			
		if (yellow) 
		{
			if (knob_down) 
			{
				if (enable_midi_drums)	{	
					ghost_parameters_t *params = ghost_note_parameters();
					params->ghost_intensity = 0.0;					
					return;
				}
			}
		}			
		return;			
	}
				
}

void stop_chord() {
	clear_chord_notes();		
	
	for (int n=0; n<6; n++) 
	{
		if (old_midinotes[n] > 0) {
			midi_send_note(0x80, old_midinotes[n], 0);	
			old_midinotes[n] = 0;
		}
		
		if (chord_notes[n] > 0) {
			midi_send_note(0x80, chord_notes[n], 0);	
			chord_notes[n] = 0;
		}				
	}

	if (voice_note != 0) midi_send_note(0x80, voice_note, 0);
}

void clear_chord_notes() {
	midi_play_chord(false, 0, 0, 0);
	if (enable_ample_guitar && ample_old_key) midi_send_note(0x80, ample_old_key, 0);				// stop ample strum		
	ample_old_key = 0;	
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
	uint8_t base = 24;	
	bool handled = false;	
		
	base = 24 + transpose;
	
	// --- F/C

	if (yellow && blue && orange && red) {
		seqtrak_chord = 4;	
		
		if (enable_style_play) midi_play_slash_chord(on, base - 12, base + 5, base + 9, base + 12);		
		chord_note = (base + 5);	
		bass_note = base - 12;
		handled = true;		
	}
	else

	// --- G/C

	if (yellow && blue && orange && green) 	{
		seqtrak_chord = 5;
		
		if (enable_style_play) midi_play_slash_chord(on, base - 12, base + 7, base + 11, base + 14);
		chord_note = (base + 7);	
		bass_note = base - 12;		
		handled = true;		
	}
	else

	// -- B

	if (red && yellow && blue && green) {
		seqtrak_chord = 0;		
		if (enable_style_play) midi_play_chord(on, base - 1, base + 3, base + 6);	
		chord_note = (base - 1);		
		handled = true;			
	}
	else

	if (red && yellow && green)     // Ab
	{	
		seqtrak_chord = 0;	
		if (enable_style_play) midi_play_chord(on, base - 4, base, base + 3);
		chord_note = (base - 4);		
		handled = true;				
	}
	else

	if (red && yellow && blue)     // A
	{
		seqtrak_chord = 0;			
		if (enable_style_play) midi_play_chord(on, base - 3, base + 13, base + 16);
		chord_note = (base - 3);		
		handled = true;		
	}
	else

	if (blue && yellow && green)     // E
	{
		seqtrak_chord = 0;			
		if (enable_style_play) midi_play_chord(on, base - 8, base + 8, base + 11);
		chord_note = (base - 8);		
		handled = true;		
	}
	else


	if (blue && red && orange)     // Eb
	{
		seqtrak_chord = 0;			
		if (enable_style_play) midi_play_chord(on, base - 9, base + 7, base + 10);
		chord_note = (base - 9);		
		handled = true;		
	}
	else

	if (yellow && blue && orange)    // F/G
	{
		seqtrak_chord = 4;		
		if (enable_style_play) midi_play_slash_chord(on, base - 17, base + 5, base + 9, base + 12);
		chord_note = (base + 5);
		bass_note = base - 17;			
		handled = true;	
	}
	else

	if (red && yellow)     // Bb
	{
		seqtrak_chord = 7;			
		if (enable_style_play) midi_play_chord(on, base - 2, base + 2, base + 5);
		chord_note = (base - 2);		
		handled = true;			
	}
	else

	if (green && yellow)     // Gsus
	{
		seqtrak_chord = 5;			
		if (enable_style_play) midi_play_chord(on, base - 5, base + 12, base + 14);
		chord_note = (base - 5);	
		chord_type = 2;
		handled = true;			
	}
	else

	if (orange && yellow)     // Csus
	{
		seqtrak_chord = 1;		
		if (enable_style_play) midi_play_chord(on, base, base + 5, base + 7);
		chord_note = (base);	
		chord_type = 2;
		handled = true;				
	}
	else

	if (yellow && blue)    // C/E
	{
		seqtrak_chord = 1;			
		if (enable_style_play) midi_play_slash_chord(on, base - 20, base, base + 4, base + 7);
		chord_note = (base);
		bass_note = base - 20;	
		handled = true;		
	}
	else

	if (green && red)     // G/B
	{
		seqtrak_chord = 5;		
		if (enable_style_play) midi_play_slash_chord(on, base - 13, base + 7, base + 11, base + 14);
		chord_note = (base + 7);
		bass_note = base - 13;			
		handled = true;	
	}
	else

	if (blue && orange)     // F/A
	{
		seqtrak_chord = 4;		
		if (enable_style_play) midi_play_slash_chord(on, base - 15, base + 5, base + 9, base + 12);
		chord_note = (base + 5);
		bass_note = base - 15;			
		handled = true;	
	}
	else

	if (green && blue)     // Em
	{
		seqtrak_chord = 3;			
		if (enable_style_play) midi_play_chord(on, base - 8, base + 7, base + 11);
		chord_note = (base - 8);	
		chord_type = 1;		
		handled = true;		
	}
	else

	if (orange && red)   // Fm
	{
		seqtrak_chord = 0;			
		if (enable_style_play) midi_play_chord(on, base - 7, base + 8, base + 12);
		chord_note = (base - 7);	
		chord_type = 1;
		handled = true;			
	}
	else

	if (green && orange)     // Gm
	{
		seqtrak_chord = 0;			
		if (enable_style_play) midi_play_chord(on, base - 5, base + 10, base + 14);
		chord_note = (base - 5);	
		chord_type = 1;
		handled = true;			
	}
	else

	if (red && blue)     // D
	{
		seqtrak_chord = 0;			
		if (enable_style_play) midi_play_chord(on, base + 2, base + 6, base + 9);
		chord_note = (base + 2);	
		handled = true;			
	}
	else

	if (yellow)    // C
	{
		seqtrak_chord = 1;		
		if (enable_style_play) midi_play_chord(on, base, base + 4, base + 7);
		chord_note = (base);	
		handled = true;	
	}
	else

	if (blue)      // Dm
	{
		seqtrak_chord = 2;			
		if (enable_style_play) midi_play_chord(on, base + 2, base + 5, base + 9);
		chord_note = (base + 2);	
		chord_type = 1;		
		handled = true;		
	}
	else

	if (orange)   // F
	{
		seqtrak_chord = 4;			
		if (enable_style_play) midi_play_chord(on, base - 7, base + 9, base + 12);
		chord_note = (base - 7);	
		handled = true;		
	}
	else

	if (green)     // G
	{
		seqtrak_chord = 5;			
		if (enable_style_play) midi_play_chord(on, base - 5, base + 11, base + 14);
		chord_note = (base - 5);			
		handled = true;		
	}
	else

	if (red)     // Am
	{
		seqtrak_chord = 6;			
		if (enable_style_play) midi_play_chord(on, base - 3, base + 12, base + 16);
		chord_note = (base - 3);	
		chord_type = 1;			
		handled = true;		
	}
	
	int O = 12;
	int C = 0, Cs = 1, Db = 1, D = 2, Ds = 3, Eb = 3, E = 4, F = 5, Fs = 6, Gb = 6, G = 7, Gs = 8, Ab = 8, A = 9, As = 10, Bb = 10, B = 11;	
	int __6th = E +O*(active_neck_pos+2), __5th = A +O*(active_neck_pos+2), __4th = D +O*(active_neck_pos+2), __3rd = G +O*(active_neck_pos+2), __2nd = B +O*(active_neck_pos+2), __1st = E +O*(active_neck_pos+3);		
	int string_frets[6] = {__6th, __5th, __4th, __3rd, __2nd, __1st};
	uint8_t chord_midinotes[6] = {0};	
	static int seq_index = 0;
	
	int string = 0;
	int notes_count = 0;
	int velocity = 110;	
	bool strum_last_chord = false;

	uint8_t note = 0;
	uint8_t ample_style_notes[8] = {36, 37, 39, 42, 44, 46, 49, 51};
	uint8_t ample_string_notes[6] = {47, 45, 43, 41, 40, 38};

	if (handled) {
		last_chord_note = chord_note;
		last_chord_type = chord_type;
	}
	
	if (active_strum_pattern > -1) 
	{	
		if (!handled && active_strum_pattern > 1 && active_neck_pos > 1) {	// play strum of last chord for ample guitar arppergio noises and non bass notes
			handled = true;
			strum_last_chord = true;
		}
		
		if (handled) 
		{
			if (up || (active_strum_pattern == 0) || strum_last_chord) 
			{										
				if (enable_ample_guitar && active_strum_pattern == 0) 
				{	
					if (style_section != old_style) {
						note = ample_style_notes[style_section] + 24;							
						midi_send_note(0x90, note, 127);				// play style key note
						ample_old_key = note;		
					}						
				} 
				else {					
					int play_pattern = strum_last_chord ? 0 : active_strum_pattern;	// select default strum for strum_last_chord
					
					while (strum_pattern[play_pattern][seq_index][0] == 0 ) {		// ignore empty pattern steps	
						seq_index++;
						if (seq_index > 11) seq_index = 0;
					}

					for (int i=0; i<6; i++) 
					{						
						mute_midinotes[i] = 0;	// reset muted notes
						
						string = 6 - strum_pattern[play_pattern][seq_index][i];
						
						if (string > -1 && string < 6) 
						{
							if (chord_chat[last_chord_note % 12][last_chord_type][string] > -1) {	// ignore unused strings
								chord_midinotes[notes_count] = string_frets[string] + chord_chat[last_chord_note % 12][last_chord_type][string];
								notes_count++;						
							}
						}
					}

					velocity = 110;
					
					if (up) {
						qsort(chord_midinotes, notes_count, sizeof(uint8_t), compUp);			
					} else {
						qsort(chord_midinotes, notes_count, sizeof(uint8_t), compDown);								
					}
				
					for (int n=0; n<notes_count; n++) {
						note = chord_midinotes[(notes_count - 1) - n];											
						if (velocity > 25) velocity = velocity - 10;
										
						if (active_neck_pos == 1) {
							if ((note % 12) > 4) note = note - 12; 	// bass needs another octave lower for bass neck pos
						}						
						
						if (enable_ample_guitar && note < 40) note = note + 12;	
						
						// don't play chord when midi drums is enabled. auto-strum will handle it on beat
						
						if (!enable_midi_drums || active_strum_pattern != 0) {
							midi_send_note(0x90, note, velocity);
						}
						
						old_midinotes[n] = note;
						mute_midinotes[n] = note;							
					}

					if (!up && enable_midi_drums && active_strum_pattern == 0) {
						// play bass note on downstroke with auto-strum
						
						note = ((bass_note ? bass_note : chord_note) % 12) + (O * (active_neck_pos + 1));
						if ((note % 12) > 4) note = note - 12;

						midi_send_program_change(0xC0, 33);						
						midi_send_note(0x90, note, 120);
						midi_send_program_change(0xC0, guitar_pc_code);	
						
						old_midinotes[0] = note;						
					}					

					seq_index++;	
					if (seq_index > 11) seq_index = 0;
				}						
				
			} else {
				note = ((bass_note ? bass_note : chord_note) % 12) + (O * (active_neck_pos + 2));
				
				if (active_strum_pattern == 1 || active_neck_pos == 1) {
					if ((note % 12) > 4) note = note - 12; 	// bass needs another octave lower for strum pattern 2 or bass
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
				else {	// handled above as strum last chord action

				}
				
				midi_send_note(0x90, note, 100);				
				old_midinotes[0] = note;
			} 
			else 
			{	
				if (enable_midi_drums && looper_status.state == LOOPER_STATE_RECORDING) 
				{					
					if (style_section % 5 == 0) {
						looper_status.current_track = 0;					// Bass Drum
						if (up) looper_status.current_track = 1;			// Snare
						looper_handle_input_internal_clock(BUTTON_EVENT_CLICK_BEGIN);					}
					else
						
					if (style_section % 5 == 1) {
						looper_status.current_track = 2;					// Closed Hit-Hat
						if (up) looper_status.current_track = 5;			// Open Hit-Hat
						looper_handle_input_internal_clock(BUTTON_EVENT_CLICK_BEGIN);						
					}	
					else
						
					if (style_section % 5 == 2) {
						looper_status.current_track = 3;					// Low floor tom
						if (up) looper_status.current_track = 6;			// High tom
						looper_handle_input_internal_clock(BUTTON_EVENT_CLICK_BEGIN);						
					}
					else
						
					if (style_section % 5 == 3) {
						looper_status.current_track = 7;					// crash cymbals
						if (up) looper_status.current_track = 8;			// ride cymbals
						looper_handle_input_internal_clock(BUTTON_EVENT_CLICK_BEGIN);						
					}
					else
						
					if (style_section % 5 == 4) {
						looper_status.current_track = 10;					// Hi Bongo
						if (up) looper_status.current_track = 11;			// Low Bongo
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

void midi_process_state(uint64_t start_us) {
	int O = 12;
	int C = 0, Cs = 1, Db = 1, D = 2, Ds = 3, Eb = 3, E = 4, F = 5, Fs = 6, Gb = 6, G = 7, Gs = 8, Ab = 8, A = 9, As = 10, Bb = 10, B = 11;	
	int __6th = E +O*(active_neck_pos+2), __5th = A +O*(active_neck_pos+2), __4th = D +O*(active_neck_pos+2), __3rd = G +O*(active_neck_pos+2), __2nd = B +O*(active_neck_pos+2), __1st = E +O*(active_neck_pos+3);		
	uint8_t auto_chord_midinotes[6] = {0};
	
	for (int i=0; i<6; i++) {
		auto_chord_midinotes[i] = mute_midinotes[i];
	}
	
	midi_current_step = (midi_current_step + 1) % 16;
	
	if (enable_midi_drums && active_strum_pattern == 0 && (enable_auto_hold || !strum_neutral)) {
		uint8_t start_action = strum_styles[style_group % 5][style_section % 5][midi_current_step][0];
		uint8_t stop_action = strum_styles[style_group % 5][style_section % 5][midi_current_step][1];
		uint8_t velocity = strum_styles[style_group % 5][style_section % 5][midi_current_step][2];
				
		// stop chord strum notes
		
		if (stop_action == 72 || stop_action == 74 || stop_action == 76 || stop_action == 79 || stop_action == 81 || stop_action == 83) 
		{		
			for (int n=0; n<6; n++) {
				midi_send_note(0x80, chord_notes[n], 0);
			}
			
		} else {
			midi_send_note(0x80, voice_note, 0);	// stop voice note
		}
		
		// play string 1 -6

		if (start_action == 62) {
			voice_note = __6th + chord_chat[last_chord_note % 12][last_chord_type][0];
			midi_send_note(0x90, voice_note, velocity);
		}
		else
			
		if (start_action == 64) {
			voice_note = __5th + chord_chat[last_chord_note % 12][last_chord_type][1];
			midi_send_note(0x90, voice_note, velocity);
		}
		else
			
		if (start_action == 65) {
			voice_note = __4th + chord_chat[last_chord_note % 12][last_chord_type][2];
			midi_send_note(0x90, voice_note, velocity);
		}
		else
			
		if (start_action == 67) {
			voice_note = __3rd + chord_chat[last_chord_note % 12][last_chord_type][3];
			midi_send_note(0x90, voice_note, velocity);
		}
		else
			
		if (start_action == 69) {
			voice_note = __2nd + chord_chat[last_chord_note % 12][last_chord_type][4];
			midi_send_note(0x90, voice_note, velocity);
		}
		else
			
		if (start_action == 71) {
			voice_note = __1st + chord_chat[last_chord_note % 12][last_chord_type][5];
			midi_send_note(0x90, voice_note, velocity);				
		}
		else

		// play chord strum up notes	
		
		if (start_action == 76 || start_action == 83) {
			qsort(auto_chord_midinotes, 6, sizeof(uint8_t), compUp);
			
			if (start_action == 83) {	// mute
				midi_send_program_change(0xC0, 28);
			}
			
			for (int n=0; n<6; n++) {
				midi_send_note(0x90, auto_chord_midinotes[n], velocity);
				chord_notes[n] = auto_chord_midinotes[n];
				if (velocity > 25) velocity = velocity - 10;
			}
			
			if (start_action == 83) {	// normal
				midi_send_program_change(0xC0, guitar_pc_code);
			}			
		} 
		else
			
		// play chord strum down notes		

		if (start_action == 72 || start_action == 74 || start_action == 79 || start_action == 81) {
			qsort(auto_chord_midinotes, 6, sizeof(uint8_t), compDown);
			
			if (start_action == 79 || start_action == 81) {	// mute
				midi_send_program_change(0xC0, 28);
			}
			
			for (int n=0; n<6; n++) {
				midi_send_note(0x90, auto_chord_midinotes[n], velocity);
				chord_notes[n] = auto_chord_midinotes[n];	
				if (velocity > 25) velocity = velocity - 10;			
			}
			
			if (start_action == 79 || start_action == 81) {	// normal
				midi_send_program_change(0xC0, guitar_pc_code);
			}			
		}
		else
		
		// play voice note		

		if (start_action == 77 || start_action == 78) {	
			voice_note = (last_chord_note % 12) + (O * (active_neck_pos + 2)); // auto_chord_midinotes[0];
			midi_send_note(0x90, voice_note, velocity);
		}		
	}
}