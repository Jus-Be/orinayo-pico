#ifndef BLE_MIDI_CONTROLLER_H_
#define BLE_MIDI_CONTROLLER_H_

#include <stdint.h>
#include <stdbool.h>

/**
 * Initialize the BLE MIDI client.  Must be called once before
 * bluetooth_init() so that BTstack subsystems are set up before
 * bluepad32 reinitialises them at startup.
 */
void ble_midi_controller_init(void);

/**
 * Start active BLE scanning for BLE-MIDI servers.
 * Connects automatically to the first device that advertises the BLE-MIDI
 * service UUID.
 */
void ble_midi_controller_scan_begin(void);

/**
 * Stop scanning.
 */
void ble_midi_controller_scan_end(void);

/**
 * Connect to a scanned MIDI peripheral by index (0-based).
 * Use after ble_midi_controller_scan_begin() has discovered devices.
 */
bool ble_midi_controller_connect(uint8_t idx);

/**
 * Disconnect from the currently connected BLE-MIDI server.
 */
void ble_midi_controller_disconnect(void);

/**
 * Returns true when connected and ready to send/receive MIDI data.
 */
bool ble_midi_controller_is_ready(void);

/**
 * Send a raw MIDI 1.0 byte stream to the connected BLE-MIDI server.
 *
 * @param data   pointer to the MIDI bytes
 * @param nbytes number of bytes to send
 * @return number of bytes accepted by the library (may be less than nbytes)
 */
uint8_t ble_midi_controller_send(const uint8_t* data, uint8_t nbytes);

/**
 * Poll for received MIDI data and dispatch it via midi_bluetooth_handle_ble_midi_byte().
 * Call this repeatedly from the main task loop.
 */
void ble_midi_controller_task(void);

#endif /* BLE_MIDI_CONTROLLER_H_ */
