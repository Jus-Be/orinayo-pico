/**
 * BLE MIDI Controller – client-side integration for Orinayo
 *
 * This module uses ble_midi_client_lib to:
 *   1. Initialise the BLE MIDI client stack (alongside Bluepad32).
 *   2. Scan for and auto-connect to any compliant BLE MIDI peripheral.
 *   3. Drain the receive ring-buffer and dispatch MIDI 1.0 messages to
 *      the application's MIDI processing functions.
 *
 * ── How to receive MIDI events ──────────────────────────────────────────────
 *
 * Fill in the template handlers below:
 *
 *   bmc_on_note_on()        – fired for every Note-On message
 *   bmc_on_note_off()       – fired for every Note-Off message (and
 *                             Note-On with velocity 0)
 *   bmc_on_control_change() – fired for every Control Change message
 *   bmc_on_program_change() – fired for every Program Change message
 *   bmc_on_pitch_bend()     – fired for every Pitch Bend message
 *   bmc_on_aftertouch()     – fired for every Channel Aftertouch message
 *   bmc_on_poly_aftertouch()– fired for every Poly Key Aftertouch message
 *   bmc_on_sysex()          – fired for complete SysEx messages
 *   bmc_on_realtime()       – fired for real-time messages (clock, start…)
 *
 * ── How to send MIDI to the connected peripheral ────────────────────────────
 *
 *   Call send_ble_midi(buffer, length) from anywhere in the application.
 *   The buffer must contain a complete MIDI 1.0 byte stream without running
 *   status.
 *
 * ── Connection management ───────────────────────────────────────────────────
 *
 *   The module automatically connects to the first BLE MIDI peripheral it
 *   discovers and reconnects after a disconnection.  If you want to target a
 *   specific device, replace the ble_midi_client_request_connect(1) call with
 *   ble_midi_client_dump_midi_peripherals() + ble_midi_client_request_connect(n).
 */

#include "ble_midi_controller.h"
#include "ble_midi_client.h"

#include "pico/stdlib.h"
#include "pico/time.h"
#include <stdio.h>

// ── Configuration ─────────────────────────────────────────────────────────

/** GAP advertising name presented by the Pico while acting as a MIDI client. */
#define BMC_PROFILE_NAME     "Orinayo"
#define BMC_PROFILE_NAME_LEN 7u

/**
 * How long (ms) to wait between auto-connect attempts when scanning.
 * Increase this if you see repeated failed connection attempts.
 */
#define BMC_RECONNECT_INTERVAL_MS 2000u

/**
 * Maximum size (bytes) of a single timestamped MIDI stream returned by
 * ble_midi_client_stream_read().  16 bytes covers any standard MIDI message
 * including short SysEx payloads; increase if you expect longer SysEx bursts.
 */
#define BMC_RX_BUF_SIZE 16u

// ── Internal state ────────────────────────────────────────────────────────

typedef enum {
    BMC_STATE_IDLE = 0,  // not yet started
    BMC_STATE_SCANNING,  // scanning for peripherals
    BMC_STATE_CONNECTING, // connection request in progress
    BMC_STATE_READY,      // connected and MIDI data flowing
} bmc_state_t;

extern bool gamepad_guitar_connected;

void process_midi_byte(uint8_t b);
void midi_send_note(uint8_t command, uint8_t note, uint8_t velocity);

static bmc_state_t bmc_state = BMC_STATE_IDLE;
static absolute_time_t bmc_reconnect_deadline;

// ── Template MIDI event handlers ──────────────────────────────────────────
//
// Fill in each function body to map incoming BLE MIDI events to Orinayo
// actions.  All parameters are standard MIDI values (0–127 unless stated).
// Functions that are unused can be left with just the (void) casts.
//

/**
 * @brief A BLE MIDI Note-On was received.
 *
 * @param channel  MIDI channel (0–15)
 * @param note     MIDI note number (0–127)
 * @param velocity Note-on velocity (1–127)
 */
static void bmc_on_note_on(uint8_t channel, uint8_t note, uint8_t velocity)
{
    (void)channel;
    (void)note;
    (void)velocity;

    // TODO: dispatch to the Orinayo MIDI pipeline.
    // Example – forward the note over USB/UART:
    //   midi_send_note(0x90u | channel, note, velocity);
}

/**
 * @brief A BLE MIDI Note-Off was received (or Note-On with velocity 0).
 *
 * @param channel  MIDI channel (0–15)
 * @param note     MIDI note number (0–127)
 * @param velocity Release velocity (0–127, often 0)
 */
static void bmc_on_note_off(uint8_t channel, uint8_t note, uint8_t velocity)
{
    (void)channel;
    (void)note;
    (void)velocity;

    // TODO: dispatch note-off.
    // Example:
    //   midi_send_note(0x80u | channel, note, velocity);
}

/**
 * @brief A BLE MIDI Control Change was received.
 *
 * @param channel    MIDI channel (0–15)
 * @param controller CC number (0–127)
 * @param value      CC value   (0–127)
 */
static void bmc_on_control_change(uint8_t channel, uint8_t controller, uint8_t value)
{
    (void)channel;
    (void)controller;
    (void)value;

    // TODO: handle CC.
    // Example:
    //   midi_send_control_change(0xB0u | channel, controller, value);
}

/**
 * @brief A BLE MIDI Program Change was received.
 *
 * @param channel MIDI channel (0–15)
 * @param program Program number (0–127)
 */
static void bmc_on_program_change(uint8_t channel, uint8_t program)
{
    (void)channel;
    (void)program;

    // TODO: handle program change.
    // Example:
    //   midi_send_program_change(0xC0u | channel, program);
}

/**
 * @brief A BLE MIDI Pitch Bend was received.
 *
 * @param channel MIDI channel (0–15)
 * @param value   14-bit signed bend value (−8192 to +8191; 0 = centre)
 */
static void bmc_on_pitch_bend(uint8_t channel, int16_t value)
{
    (void)channel;
    (void)value;

    // TODO: handle pitch bend.
    // The raw two-byte BLE MIDI payload (lsb, msb) is encoded as:
    //   value = ((msb & 0x7F) << 7) | (lsb & 0x7F) - 8192
}

/**
 * @brief A BLE MIDI Channel Aftertouch (Channel Pressure) was received.
 *
 * @param channel  MIDI channel (0–15)
 * @param pressure Pressure value (0–127)
 */
static void bmc_on_aftertouch(uint8_t channel, uint8_t pressure)
{
    (void)channel;
    (void)pressure;

    // TODO: handle channel aftertouch.
}

/**
 * @brief A BLE MIDI Polyphonic Key Aftertouch was received.
 *
 * @param channel  MIDI channel (0–15)
 * @param note     Note number the pressure applies to (0–127)
 * @param pressure Pressure value (0–127)
 */
static void bmc_on_poly_aftertouch(uint8_t channel, uint8_t note, uint8_t pressure)
{
    (void)channel;
    (void)note;
    (void)pressure;

    // TODO: handle polyphonic aftertouch.
}

/**
 * @brief A complete BLE MIDI SysEx message was received.
 *
 * The buffer includes the leading 0xF0 and trailing 0xF7 bytes.
 *
 * @param buf   Pointer to the SysEx byte array.
 * @param nbytes Total length including 0xF0 and 0xF7.
 */
static void bmc_on_sysex(const uint8_t *buf, uint8_t nbytes)
{
    (void)buf;
    (void)nbytes;

    // TODO: handle SysEx.
    // Example – forward verbatim:
    //   midi_n_stream_write(0, 0, buf, nbytes);
}

/**
 * @brief A BLE MIDI real-time message was received.
 *
 * Real-time messages are single-byte and time-critical.  They may appear
 * between any two bytes of a regular MIDI message.
 *
 * Common real-time status bytes:
 *   0xF8 – MIDI Timing Clock
 *   0xFA – MIDI Start
 *   0xFB – MIDI Continue
 *   0xFC – MIDI Stop
 *   0xFE – Active Sensing
 *   0xFF – System Reset
 *
 * @param status The real-time status byte (0xF8–0xFF).
 */
static void bmc_on_realtime(uint8_t status)
{
    (void)status;

    // TODO: handle real-time message.
    // Example:
    //   switch (status) {
    //       case 0xFA: midi_start_stop(true);  break;  // Start
    //       case 0xFC: midi_start_stop(false); break;  // Stop
    //       default: break;
    //   }
}

// ── MIDI parser ───────────────────────────────────────────────────────────

/**
 * Parse a raw MIDI 1.0 byte stream (no running status, as returned by
 * ble_midi_client_stream_read) and dispatch the message to the handler above.
 *
 * @param buf    Buffer returned by ble_midi_client_stream_read().
 * @param nbytes Number of valid bytes in buf.
 */
static void bmc_dispatch_midi(const uint8_t *buf, uint8_t nbytes)
{
    if (nbytes == 0) return;
	for (uint8_t i=0; i<nbytes; i++) process_midi_byte(buf[i]);

    uint8_t status = buf[0];

    // ── Real-time: single byte, 0xF8–0xFF ───────────────────────────────
    if (status >= 0xF8u) {
        bmc_on_realtime(status);
        return;
    }

    // ── SysEx: 0xF0 … 0xF7 ─────────────────────────────────────────────
    if (status == 0xF0u) {
        bmc_on_sysex(buf, nbytes);
        return;
    }

    // ── Channel messages ─────────────────────────────────────────────────
    uint8_t type    = status & 0xF0u;
    uint8_t channel = status & 0x0Fu;

    switch (type) {
        case 0x80u: // Note Off
            if (nbytes >= 3u)
                bmc_on_note_off(channel, buf[1], buf[2]);
            break;

        case 0x90u: // Note On (velocity 0 → Note Off)
            if (nbytes >= 3u) {
                if (buf[2] == 0u)
                    bmc_on_note_off(channel, buf[1], 0u);
                else
                    bmc_on_note_on(channel, buf[1], buf[2]);
            }
            break;

        case 0xA0u: // Polyphonic Key Aftertouch
            if (nbytes >= 3u)
                bmc_on_poly_aftertouch(channel, buf[1], buf[2]);
            break;

        case 0xB0u: // Control Change
            if (nbytes >= 3u)
                bmc_on_control_change(channel, buf[1], buf[2]);
            break;

        case 0xC0u: // Program Change
            if (nbytes >= 2u)
                bmc_on_program_change(channel, buf[1]);
            break;

        case 0xD0u: // Channel Aftertouch
            if (nbytes >= 2u)
                bmc_on_aftertouch(channel, buf[1]);
            break;

        case 0xE0u: // Pitch Bend
            if (nbytes >= 3u) {
                int16_t bend = (int16_t)(((uint16_t)(buf[2] & 0x7Fu) << 7u) |
                                          (uint16_t)(buf[1] & 0x7Fu)) - 8192;
                bmc_on_pitch_bend(channel, bend);
            }
            break;

        default:
            // Unknown or unhandled system-common message – ignore.
            break;
    }
}

// ── Public API ────────────────────────────────────────────────────────────

void ble_midi_controller_init(void)
{
    bmc_state = BMC_STATE_IDLE;

    // Initialise the BLE MIDI client library.  This registers BTStack event
    // handlers and prepares the packet codec.  Call before uni_init() so that
    // the handlers are in the chain when BTStack first reaches HCI_STATE_WORKING.
    ble_midi_client_init(BMC_PROFILE_NAME, (uint8_t)BMC_PROFILE_NAME_LEN,
                         IO_CAPABILITY_NO_INPUT_NO_OUTPUT,
                         SM_AUTHREQ_NO_BONDING);
}

void ble_midi_controller_scan_begin(void)
{
    // When ble_midi_client_init() was called before uni_init(), the internal
    // state stays at BLEMC_DEINIT because HCI_STATE_WORKING is processed by
    // Bluepad32 before our handler has a chance to act on it.  Promote the
    // state to IDLE so that ble_midi_client_scan_begin() takes the direct
    // gap_start_scan() path rather than trying to power the controller on again.
    ble_midi_client_promote_to_idle();
    ble_midi_client_scan_begin();

    bmc_state = BMC_STATE_SCANNING;
    bmc_reconnect_deadline = make_timeout_time_ms(BMC_RECONNECT_INTERVAL_MS);

    printf("[BLE MIDI] Scanning for peripherals...\n");
}

bool ble_midi_controller_is_ready(void)
{
    return ble_midi_client_is_ready();
}

void ble_midi_controller_poll(void)
{	
	if (gamepad_guitar_connected) return;
		
    switch (bmc_state) {
        case BMC_STATE_IDLE:
            // Not started yet – nothing to do.
            break;

        case BMC_STATE_SCANNING:

            // Once the reconnect deadline expires, attempt to auto-connect to
            // the first BLE MIDI peripheral discovered during the scan.
            if (!ble_midi_client_waiting_for_connection() && !ble_midi_client_is_connected() && time_reached(bmc_reconnect_deadline))  {
				midi_send_note(0x91, 1, 1);
				
                // ble_midi_client_request_connect(1) targets the first entry
                // in the discovered-devices list (1-based index).  Returns
                // false when the list is still empty.
                if (ble_midi_client_request_connect(1u)) {
					midi_send_note(0x92, 2, 2);					
					
                    printf("[BLE MIDI] Connecting to first discovered peripheral...\n");
                    bmc_state = BMC_STATE_CONNECTING;
                } else {
					midi_send_note(0x93, 3, 3);					
                    // No peripheral found yet – retry after another interval.
                    bmc_reconnect_deadline =
                        make_timeout_time_ms(BMC_RECONNECT_INTERVAL_MS);
                }
            }
            break;

        case BMC_STATE_CONNECTING:
			
            if (ble_midi_client_is_ready()) {
				midi_send_note(0x95, 5, 5);					
                printf("[BLE MIDI] Connected and ready.\n");
                bmc_state = BMC_STATE_READY;
            } else if (!ble_midi_client_is_connected() && !ble_midi_client_waiting_for_connection()) {
				midi_send_note(0x96, 6, 6);					
                // The connection attempt was rejected or timed out.
                printf("[BLE MIDI] Connection failed. Resuming scan.\n");
                ble_midi_controller_scan_begin();
            }
            break;

        case BMC_STATE_READY:
			
            if (!ble_midi_client_is_connected()) {
				midi_send_note(0x97, 7, 7);					
                printf("[BLE MIDI] Disconnected. Resuming scan.\n");
                ble_midi_controller_scan_begin();
                break;
            }

            // Drain all timestamped MIDI streams available in the ring-buffer.
            {
                uint8_t  buf[BMC_RX_BUF_SIZE];
                uint16_t timestamp;
                uint8_t  nbytes;

                while ((nbytes = ble_midi_client_stream_read((uint8_t)sizeof(buf), buf, &timestamp)) > 0u) {
                    bmc_dispatch_midi(buf, nbytes);
                }
            }
            break;
    }
}
