/**
 * ring_buffer_lib_config.h
 * Configuration for ring_buffer_lib used by ble_midi_lib.
 *
 * The RING_BUFFER_SIZE_TYPE must be at least 16-bit to accommodate
 * the large ring buffers needed for Bluetooth MIDI packets.
 */
#ifndef RING_BUFFER_LIB_CONFIG_H
#define RING_BUFFER_LIB_CONFIG_H

#define RING_BUFFER_SIZE_TYPE uint16_t

#endif /* RING_BUFFER_LIB_CONFIG_H */
