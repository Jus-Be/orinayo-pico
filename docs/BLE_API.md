# BLE Client API Reference

Quick reference for the BLE client API functions provided by the Orinayo Pico project.

## Core Functions

### `bool bluetooth_init(void)`
Initializes the Bluetooth subsystem with BLE client functionality.

**Returns:** `true` on success, `false` on failure

**Usage:**
```c
if (!bluetooth_init()) {
    printf("Bluetooth initialization failed!\n");
    // Handle error
}
```

**Details:**
- Sets up bluepad32 library
- Configures BLE client mode
- Starts automatic device scanning
- Must be called before any other BLE functions

---

## Status Functions

### `bool bluetooth_is_connected(void)`
Checks if any BLE device is currently connected.

**Returns:** `true` if connected, `false` otherwise

**Usage:**
```c
if (bluetooth_is_connected()) {
    printf("BLE device is connected\n");
} else {
    printf("No BLE devices connected\n");
}
```

### `uint8_t bluetooth_get_connected_device_count(void)`
Gets the number of currently connected BLE devices.

**Returns:** Number of connected devices (0 to `CONFIG_BLUEPAD32_MAX_DEVICES`)

**Usage:**
```c
uint8_t count = bluetooth_get_connected_device_count();
printf("Connected devices: %d\n", count);
```

### `bool bluetooth_is_scanning(void)`
Checks if BLE scanning is currently active.

**Returns:** `true` if scanning, `false` otherwise

**Usage:**
```c
if (bluetooth_is_scanning()) {
    printf("Scanning for BLE devices...\n");
}
```

---

## Control Functions

### `void bluetooth_start_scanning(void)`
Manually start BLE device scanning.

**Usage:**
```c
bluetooth_start_scanning();
printf("Started BLE scanning\n");
```

**Notes:**
- Automatically called during initialization
- Safe to call if already scanning
- LED will turn on to indicate scanning
- Only works after `bluetooth_init()` completes

### `void bluetooth_stop_scanning(void)`
Manually stop BLE device scanning.

**Usage:**
```c
bluetooth_stop_scanning();
printf("Stopped BLE scanning\n");
```

**Notes:**
- Automatically called when a device connects
- Saves power when not actively looking for devices
- Safe to call if not scanning

### `void bluetooth_clear_bonded_devices(void)`
Clears all stored BLE bonding keys.

**Usage:**
```c
bluetooth_clear_bonded_devices();
printf("Cleared all bonded devices - fresh pairing required\n");
```

**Notes:**
- Forces all devices to re-pair on next connection
- Useful for troubleshooting connection issues
- Called automatically during initialization

---

## Usage Patterns

### Basic Connection Monitoring
```c
#include "pico_bluetooth.h"

void check_bluetooth_status(void) {
    if (bluetooth_is_connected()) {
        uint8_t count = bluetooth_get_connected_device_count();
        printf("Connected devices: %d\n", count);
    } else if (bluetooth_is_scanning()) {
        printf("Scanning for devices...\n");
    } else {
        printf("Bluetooth idle\n");
    }
}
```

### Manual Connection Control
```c
void reset_bluetooth_connection(void) {
    // Clear bonded devices for fresh pairing
    bluetooth_clear_bonded_devices();
    
    // Stop scanning
    bluetooth_stop_scanning();
    
    // Wait a moment
    sleep_ms(1000);
    
    // Restart scanning
    bluetooth_start_scanning();
    
    printf("Bluetooth reset - ready for new connections\n");
}
```

### Power-Aware Scanning
```c
void manage_bluetooth_power(bool power_save_mode) {
    if (power_save_mode && !bluetooth_is_connected()) {
        // Stop scanning to save power when not connected
        bluetooth_stop_scanning();
    } else if (!power_save_mode && !bluetooth_is_scanning()) {
        // Resume scanning when not in power save mode
        bluetooth_start_scanning();
    }
}
```

### Connection State Machine
```c
typedef enum {
    BT_STATE_INIT,
    BT_STATE_SCANNING,
    BT_STATE_CONNECTED,
    BT_STATE_ERROR
} bt_state_t;

bt_state_t get_bluetooth_state(void) {
    static bool initialized = false;
    
    if (!initialized) {
        return BT_STATE_INIT;
    } else if (bluetooth_is_connected()) {
        return BT_STATE_CONNECTED;
    } else if (bluetooth_is_scanning()) {
        return BT_STATE_SCANNING;
    } else {
        return BT_STATE_ERROR;
    }
}

void handle_bluetooth_state(void) {
    switch (get_bluetooth_state()) {
        case BT_STATE_INIT:
            if (bluetooth_init()) {
                printf("Bluetooth initialized\n");
            }
            break;
            
        case BT_STATE_SCANNING:
            printf("Scanning for devices...\n");
            break;
            
        case BT_STATE_CONNECTED:
            printf("Device connected\n");
            break;
            
        case BT_STATE_ERROR:
            printf("Bluetooth error - restarting scan\n");
            bluetooth_start_scanning();
            break;
    }
}
```

---

## Configuration Constants

These constants can be modified in `sdkconfig.h`:

### `CONFIG_BLUEPAD32_MAX_DEVICES`
Maximum number of simultaneous BLE connections.
- Default: `1`
- Range: `1-4`
- Higher values increase memory usage

### `CONFIG_BLUEPAD32_GAP_SECURITY`
Enable BLE security and pairing.
- Default: `1` (enabled)
- Set to `0` to disable pairing (faster but less secure)

### `CONFIG_BLUEPAD32_ENABLE_BLE_BY_DEFAULT`
Enable BLE functionality at startup.
- Default: `1` (enabled)
- Set to `0` to disable BLE entirely

---

## LED Indicators

The onboard LED provides visual feedback for BLE status:

| LED State | Meaning |
|-----------|---------|
| ON (solid) | Scanning for devices |
| OFF | Device connected and ready |
| FLASHING | Activity (button presses, data transfer) |
| RAPID BLINK | Error condition |

---

## Error Handling

### Common Error Scenarios

1. **Initialization Failure:**
```c
if (!bluetooth_init()) {
    // Hardware issue or insufficient memory
    // Check power supply and reset device
}
```

2. **Connection Lost:**
```c
// Monitor connection status
static bool was_connected = false;
bool is_connected = bluetooth_is_connected();

if (was_connected && !is_connected) {
    printf("Device disconnected - restarting scan\n");
    bluetooth_start_scanning();
}
was_connected = is_connected;
```

3. **Pairing Issues:**
```c
// Clear bonded devices if connection repeatedly fails
static int connection_attempts = 0;

if (!bluetooth_is_connected() && bluetooth_is_scanning()) {
    connection_attempts++;
    if (connection_attempts > 10) {
        printf("Clearing bonded devices after repeated failures\n");
        bluetooth_clear_bonded_devices();
        connection_attempts = 0;
    }
}
```

---

## Thread Safety

All BLE API functions are designed to be called from the main thread. The bluepad32 library handles BLE operations in the background, so these functions primarily control state and provide status information.

**Safe to call from main loop:**
- All status functions (`bluetooth_is_*`, `bluetooth_get_*`)
- All control functions (`bluetooth_start_*`, `bluetooth_stop_*`, `bluetooth_clear_*`)

**Not thread-safe:**
- Do not call from interrupt handlers
- Do not call from FreeRTOS tasks without proper synchronization