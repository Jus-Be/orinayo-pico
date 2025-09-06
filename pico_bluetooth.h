#ifndef PICO_BLUETOOTH_H_
#define PICO_BLUETOOTH_H_

#include <stdbool.h>
#include <stdint.h>

/**
 * Initialize the Bluetooth subsystem with BLE client functionality.
 * This sets up bluepad32 and starts scanning for BLE HID devices.
 * 
 * @return true on success, false on failure
 */
bool bluetooth_init(void);

/**
 * Get the current BLE connection status.
 * 
 * @return true if a BLE device is connected, false otherwise
 */
bool bluetooth_is_connected(void);

/**
 * Get the number of connected BLE devices.
 * 
 * @return Number of connected devices (0-CONFIG_BLUEPAD32_MAX_DEVICES)
 */
uint8_t bluetooth_get_connected_device_count(void);

/**
 * Start BLE scanning for devices.
 * This is called automatically during initialization but can be used
 * to restart scanning if needed.
 */
void bluetooth_start_scanning(void);

/**
 * Stop BLE scanning for devices.
 * This is typically called automatically when a device connects.
 */
void bluetooth_stop_scanning(void);

/**
 * Check if BLE scanning is currently active.
 * 
 * @return true if scanning, false otherwise
 */
bool bluetooth_is_scanning(void);

/**
 * Clear all stored BLE bonding keys.
 * This forces all devices to re-pair on next connection.
 */
void bluetooth_clear_bonded_devices(void);

#endif  // PICO_BLUETOOTH_H_