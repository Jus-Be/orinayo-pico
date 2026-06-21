#ifndef BLE_MIDI_CONTROLLER_H_
#define BLE_MIDI_CONTROLLER_H_

#include <stdbool.h>
#include <stdint.h>

/**
 * BLE MIDI Controller – client-side interface for Orinayo
 *
 * This module wraps ble_midi_client_lib to scan for, connect to, and receive
 * MIDI data from any compliant BLE MIDI peripheral (keyboard, pad, wind
 * controller, etc.).
 *
 * Typical call sequence
 * ---------------------
 *  1. bluetooth_init()  -> ble_midi_controller_init()  (before uni_init)
 *  2. on_init_complete  -> ble_midi_controller_scan_begin()
 *  3. main loop         -> ble_midi_controller_poll()  (every iteration)
 *  4. outgoing MIDI     -> send_ble_midi(buf, len)
 */

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Initialise the BLE MIDI client stack.
 *
 * Registers BTStack event handlers and allocates codec resources.  Must be
 * called once before uni_init() so that handlers are in place when BTStack
 * first raises HCI_STATE_WORKING.
 */
void ble_midi_controller_init(void);

/**
 * @brief Start scanning for BLE MIDI peripherals and auto-connect.
 *
 * Call from the Bluepad32 on_init_complete callback, or any other point after
 * BTStack has reached HCI_STATE_WORKING.  Handles the case where
 * HCI_STATE_WORKING was already processed before ble_midi_client_init()
 * registered its handler by promoting the internal state to IDLE first.
 */
void ble_midi_controller_scan_begin(void);

/**
 * @brief Poll for incoming BLE MIDI events.
 *
 * Drains the receive ring-buffer, parses each MIDI 1.0 message, and calls the
 * appropriate skeleton handler (see ble_midi_controller.c).  Also manages the
 * connection state machine (scanning → connecting → ready → reconnect on
 * disconnect).
 *
 * Call frequently from the application's main loop.
 */
void ble_midi_controller_poll(void);

/**
 * @brief Returns true when a BLE MIDI peripheral is connected and the MIDI
 * data channel is ready to use.
 */
bool ble_midi_controller_is_ready(void);

#ifdef __cplusplus
}
#endif

#endif  // BLE_MIDI_CONTROLLER_H_
