#include "ble_midi_controller.h"

#include "ble_midi_lib/ble_midi_client.h"
#include "ble_midi_lib/ble_midi_pkt_codec.h"

#include "bluetooth.h"         /* io_capability_t */
#include "sm.h"                /* SM_AUTHREQ_* */

/* Forward declaration: defined in pico_bluetooth.c.
 * Called once for every complete MIDI message received over BLE-MIDI. */
void midi_bluetooth_handle_ble_midi_byte(uint8_t midi_byte);

/* -------------------------------------------------------------------------
 * Init
 * ---------------------------------------------------------------------- */

void ble_midi_controller_init(void)
{
    /* IO_CAPABILITY_NO_INPUT_NO_OUTPUT: Just Works pairing – no PIN entry.
     * 0 for secmask: no bonding required (matches most MIDI controllers). */
    ble_midi_client_init("Orinayo", 7,
                         IO_CAPABILITY_NO_INPUT_NO_OUTPUT,
                         0);
}

/* -------------------------------------------------------------------------
 * Scan / connect / disconnect
 * ---------------------------------------------------------------------- */

void ble_midi_controller_scan_begin(void)
{
    ble_midi_client_scan_begin();
}

void ble_midi_controller_scan_end(void)
{
    ble_midi_client_scan_end();
}

bool ble_midi_controller_connect(uint8_t idx)
{
    return ble_midi_client_request_connect(idx);
}

void ble_midi_controller_disconnect(void)
{
    ble_midi_client_request_disconnect();
}

bool ble_midi_controller_is_ready(void)
{
    return ble_midi_client_is_ready();
}

/* -------------------------------------------------------------------------
 * Send MIDI to the connected server
 * ---------------------------------------------------------------------- */

uint8_t ble_midi_controller_send(const uint8_t* data, uint8_t nbytes)
{
    if (!ble_midi_client_is_ready())
        return 0;
    return ble_midi_client_stream_write(nbytes, data);
}

/* -------------------------------------------------------------------------
 * Receive task – drain the decoded MIDI ring buffer and forward each byte
 * ---------------------------------------------------------------------- */

void ble_midi_controller_task(void)
{
    if (!ble_midi_client_is_ready())
        return;

    uint8_t  midi_buf[4];   /* largest single BLE-MIDI message is 3 bytes */
    uint16_t timestamp;
    uint8_t  nbytes;

    /* ble_midi_client_stream_read() returns one complete MIDI message per
     * call; loop until the ring buffer is empty. */
    while ((nbytes = ble_midi_client_stream_read(sizeof(midi_buf),
                                                  midi_buf,
                                                  &timestamp)) > 0) {
        for (uint8_t i = 0; i < nbytes; i++) {
            midi_bluetooth_handle_ble_midi_byte(midi_buf[i]);
        }
    }
}
