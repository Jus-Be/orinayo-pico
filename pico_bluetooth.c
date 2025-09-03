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

#ifndef CONFIG_BLUEPAD32_PLATFORM_CUSTOM
#error "Pico W must use BLUEPAD32_PLATFORM_CUSTOM"
#endif


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
	 cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, false);
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
  // PICO_DEBUG("[BT] Restarted scanning (device disconnected)\n");
}

static uni_error_t pico_bluetooth_on_device_ready(uni_hid_device_t* d) {
  // You can reject the connection by returning an error.
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

  switch (ctl->klass) {
    case UNI_CONTROLLER_CLASS_GAMEPAD:
      // Print device Id and dump gamepad.
		cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, true);
		
		uint16_t buttons = ctl->gamepad.buttons;
		uint16_t misc_buttons = ctl->gamepad.misc_buttons;
				
		uint8_t but0 = (buttons >> 0) & 0x01;
		uint8_t but1 = (buttons >> 1) & 0x01;
		uint8_t but2 = (buttons >> 2) & 0x01;
		uint8_t but3 = (buttons >> 3) & 0x01;
		uint8_t but4 = (buttons >> 4) & 0x01;
		uint8_t but5 = (buttons >> 5) & 0x01;
		uint8_t but6 = (buttons >> 6) & 0x01;
		uint8_t but7 = (buttons >> 7) & 0x01;
		uint8_t but8 = (buttons >> 8) & 0x01;
		uint8_t but9 = (buttons >> 9) & 0x01;
		uint8_t butA = (buttons >> 10) & 0x01;
		uint8_t butB = (buttons >> 11) & 0x01;	
		uint8_t butC = (buttons >> 12) & 0x01;
		uint8_t butD = (buttons >> 13) & 0x01;	
		uint8_t butE = (buttons >> 14) & 0x01;
		uint8_t butF = (buttons >> 15) & 0x01;	

		uint8_t dpad_left = ctl->gamepad.dpad & 0x08;	
		uint8_t dpad_right = ctl->gamepad.dpad & 0x04;
		uint8_t dpad_up = ctl->gamepad.dpad & 0x01;
		uint8_t dpad_down = ctl->gamepad.dpad & 0x02;
		
		if (but0) midi_send_note(0x80,  but0, 0);
		if (but1) midi_send_note(0x81,  but1, 1);		
		if (but2) midi_send_note(0x82,  but2, 2);
		if (but3) midi_send_note(0x83,  but3, 3);
		if (but4) midi_send_note(0x84,  but4, 4);
		if (but5) midi_send_note(0x85,  but5, 5);	
		if (but6) midi_send_note(0x86,  but6, 6);
		if (but7) midi_send_note(0x87,  but7, 7);		
		if (but8) midi_send_note(0x88,  but8, 8);
		if (but9) midi_send_note(0x89,  but9, 9);
		if (butA) midi_send_note(0x8A,  butA, 10);
		if (butB) midi_send_note(0x8B,  butB, 11);	
		
		if (dpad_left) midi_send_note(0x8C,  dpad_left, 0);
		if (dpad_right) midi_send_note(0x8D,  dpad_right, 0);
		if (dpad_up) midi_send_note(0x8E,  dpad_up, 0);
		if (dpad_down) midi_send_note(0x8F,  dpad_down, 0);

		uint8_t mbut0 = (misc_buttons >> 0) & 0x01;
		uint8_t mbut1 = (misc_buttons >> 1) & 0x01;
		uint8_t mbut2 = (misc_buttons >> 2) & 0x01;
		uint8_t mbut3 = (misc_buttons >> 3) & 0x01;
		uint8_t mbut4 = (misc_buttons >> 4) & 0x01;
		uint8_t mbut5 = (misc_buttons >> 5) & 0x01;
		uint8_t mbut6 = (misc_buttons >> 6) & 0x01;
		uint8_t mbut7 = (misc_buttons >> 7) & 0x01;
		uint8_t mbut8 = (misc_buttons >> 8) & 0x01;
		uint8_t mbut9 = (misc_buttons >> 9) & 0x01;
		uint8_t mbutA = (misc_buttons >> 10) & 0x01;
		uint8_t mbutB = (misc_buttons >> 11) & 0x01;	
		uint8_t mbutC = (misc_buttons >> 12) & 0x01;
		uint8_t mbutD = (misc_buttons >> 13) & 0x01;	
		uint8_t mbutE = (misc_buttons >> 14) & 0x01;
		uint8_t mbutF = (misc_buttons >> 15) & 0x01;			
		uint8_t mbutG = (misc_buttons >> 16) & 0x01;	
		uint8_t mbutH = (misc_buttons >> 17) & 0x01;			
		uint8_t mbutI = (misc_buttons >> 18) & 0x01;
		
		if (mbut0) midi_send_note(0x90, mbut0, 0);
		if (mbut1) midi_send_note(0x91, mbut1, 1);		
		if (mbut2) midi_send_note(0x92, mbut2, 2);	
		if (mbut3) midi_send_note(0x93,  mbut3, 3);
		if (mbut4) midi_send_note(0x94,  mbut4, 4);
		if (mbut5) midi_send_note(0x95,  mbut5, 5);	
		if (mbut6) midi_send_note(0x96,  mbut6, 6);
		if (mbut7) midi_send_note(0x97,  mbut7, 7);		
		if (mbut8) midi_send_note(0x98,  mbut8, 8);
		if (mbut9) midi_send_note(0x99,  mbut9, 9);
		if (mbutA) midi_send_note(0x9A,  mbutA, 10);
		if (mbutB) midi_send_note(0x9B,  mbutB, 11);			
		if (mbutC) midi_send_note(0x9C,  mbutC, 12);
		if (mbutD) midi_send_note(0x9D,  mbutD, 13);
		if (mbutE) midi_send_note(0x9E,  mbutE, 14);
		if (mbutF) midi_send_note(0x9F,  mbutF, 15);		
		if (mbutG) midi_send_note(0x9F,  mbutG, 16);
		if (mbutH) midi_send_note(0x9F,  mbutH, 17);		
		if (mbutI) midi_send_note(0x9F,  mbutI, 18);
		
		uint8_t axis_x = ctl->gamepad.axis_x / 4;
		uint8_t axis_y = ctl->gamepad.axis_y / 4;
		uint8_t axis_rx = ctl->gamepad.axis_rx / 4;
		uint8_t axis_ry = ctl->gamepad.axis_ry / 4;	

		if (axis_x) midi_send_note(0x8F,  axis_x, 0);	
		if (axis_y) midi_send_note(0x8F,  axis_y, 0);	
		if (axis_rx) midi_send_note(0x8F,  axis_rx, 0);	
		if (axis_ry) midi_send_note(0x8F,  axis_ry, 0);		
	
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