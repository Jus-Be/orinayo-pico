# BLE Client Configuration Guide

This guide explains how to configure and customize the BLE client functionality in the Orinayo Pico project.

## Configuration Files

### 1. `sdkconfig.h` - Main Configuration

```c
#define CONFIG_BLUEPAD32_PLATFORM_CUSTOM           // Use custom platform
#define CONFIG_TARGET_PICO_W                       // Target Pico W hardware
#define CONFIG_BLUEPAD32_LOG_LEVEL 0               // Log level (0=INFO, 1=DEBUG, 2=VERBOSE)
#define CONFIG_BLUEPAD32_MAX_DEVICES 1             // Maximum concurrent connections
#define CONFIG_BLUEPAD32_MAX_ALLOWLIST 1           // Maximum devices in allowlist
#define CONFIG_BLUEPAD32_GAP_SECURITY 1            // Enable pairing/security
#define CONFIG_BLUEPAD32_ENABLE_BLE_BY_DEFAULT 1   // Enable BLE at startup
```

### 2. `btstack_config.h` - BTStack Configuration

```c
// BLE Features
#define ENABLE_LE_CENTRAL                          // Enable BLE central role
#define ENABLE_GATT_CLIENT_PAIRING                 // Enable GATT pairing
#define ENABLE_LE_SECURE_CONNECTIONS               // Enable secure connections
#define ENABLE_LE_PRIVACY_ADDRESS_RESOLUTION       // Enable address resolution

// Connection Limits
#define MAX_NR_HCI_CONNECTIONS 4                   // Max HCI connections
#define MAX_NR_HIDS_CLIENTS 4                      // Max HID service clients
#define MAX_NR_LE_DEVICE_DB_ENTRIES 16             // Max bonded devices
```

## Customization Options

### Device Filtering

In `pico_bluetooth.c`, the `pico_bluetooth_on_device_discovered()` function filters devices:

```c
// Accept only HID devices (gamepads, keyboards, mice)
if (((cod & UNI_BT_COD_MINOR_MASK) & UNI_BT_COD_MINOR_KEYBOARD) == UNI_BT_COD_MINOR_KEYBOARD) {
    return UNI_ERROR_IGNORE_DEVICE;  // Ignore keyboards
}

// Or accept specific device names
if (strstr(name, "Xbox Controller")) {
    return UNI_ERROR_SUCCESS;        // Accept Xbox controllers
}
```

### Connection Behavior

Modify scanning behavior in the connection callbacks:

```c
static void pico_bluetooth_on_device_connected(uni_hid_device_t* d) {
    // Option 1: Stop scanning when any device connects (default)
    uni_bt_stop_scanning_safe();
    
    // Option 2: Continue scanning for multiple devices
    // if (bt_connected_device_count < CONFIG_BLUEPAD32_MAX_DEVICES) {
    //     // Keep scanning for more devices
    // } else {
    //     uni_bt_stop_scanning_safe();
    // }
}
```

### Security Settings

Adjust security requirements in `uni_bt_le.c` (bluepad32 library):

```c
// In uni_bt_le_setup():

// Option 1: No security (fastest connection)
sm_set_authentication_requirements(0);

// Option 2: Bonding only (default)
sm_set_authentication_requirements(SM_AUTHREQ_BONDING);

// Option 3: Secure connections + bonding (most secure)
sm_set_authentication_requirements(SM_AUTHREQ_SECURE_CONNECTION | SM_AUTHREQ_BONDING);
```

## Performance Tuning

### Scan Parameters

Adjust BLE scanning in `uni_bt_le.c`:

```c
// In uni_bt_le_setup():
// gap_set_scan_parameters(type, interval, window)
gap_set_scan_parameters(0,    // 0=passive, 1=active scanning
                       48,    // Scan interval (30ms units)
                       48);   // Scan window (30ms units)

// For faster discovery:
gap_set_scan_parameters(1, 32, 32);  // Active scan, 20ms

// For power saving:
gap_set_scan_parameters(0, 96, 48);  // Passive scan, longer interval
```

### Connection Intervals

Modify connection parameters for different use cases:

```c
// In connection event handler, you can request parameter updates:
// gap_request_connection_parameter_update(con_handle, min_interval, max_interval, latency, timeout);

// Gaming (low latency):
gap_request_connection_parameter_update(con_handle, 6, 10, 0, 100);

// Audio/MIDI (balanced):
gap_request_connection_parameter_update(con_handle, 8, 16, 0, 200);

// Low power (high latency):
gap_request_connection_parameter_update(con_handle, 40, 80, 4, 500);
```

## Device-Specific Configuration

### Xbox Controllers

```c
// In device discovery filter:
if (strstr(name, "Xbox") || strstr(name, "Controller")) {
    printf("Xbox controller detected\n");
    return UNI_ERROR_SUCCESS;
}
```

### Nintendo Switch Pro Controller

```c
if (strstr(name, "Pro Controller")) {
    printf("Switch Pro controller detected\n");
    return UNI_ERROR_SUCCESS;
}
```

### Generic HID Devices

```c
// Filter by device appearance in advertising data
if (appearance == UNI_BT_HID_APPEARANCE_GAMEPAD ||
    appearance == UNI_BT_HID_APPEARANCE_JOYSTICK) {
    return UNI_ERROR_SUCCESS;
}
```

## Debugging Configuration

### Enable Debug Output

1. Modify `sdkconfig.h`:
```c
#define CONFIG_BLUEPAD32_LOG_LEVEL 2  // Verbose logging
```

2. Enable BTStack debug in `btstack_config.h`:
```c
#define ENABLE_LOG_DEBUG
#define ENABLE_LOG_VERBOSE
```

3. Enable stdio in your application:
```c
pico_enable_stdio_uart(your_target 1)  // Enable UART output
pico_enable_stdio_usb(your_target 0)   // Disable USB (if using for MIDI)
```

### Debug Functions

Use the new API functions for runtime debugging:

```c
#include "pico_bluetooth.h"

// Check connection status
if (bluetooth_is_connected()) {
    printf("Device connected, count: %d\n", bluetooth_get_connected_device_count());
}

// Check scanning status
if (bluetooth_is_scanning()) {
    printf("Scanning for devices...\n");
}

// Manual control
bluetooth_stop_scanning();
sleep_ms(5000);
bluetooth_start_scanning();
```

## Power Management

### Optimize for Battery Operation

1. Reduce scan frequency:
```c
gap_set_scan_parameters(0, 160, 48);  // Scan every 100ms for 30ms
```

2. Use longer connection intervals:
```c
gap_request_connection_parameter_update(con_handle, 40, 80, 4, 500);
```

3. Disable unnecessary features in `btstack_config.h`:
```c
// Comment out if not needed:
// #define ENABLE_LE_DATA_LENGTH_EXTENSION
// #define ENABLE_LE_PRIVACY_ADDRESS_RESOLUTION
```

### Optimize for Performance

1. Aggressive scanning:
```c
gap_set_scan_parameters(1, 32, 32);   // Fast active scanning
```

2. Low latency connections:
```c
gap_request_connection_parameter_update(con_handle, 6, 10, 0, 100);
```

## Troubleshooting

### Common Issues and Solutions

**Device not discovered:**
- Increase scan window: `gap_set_scan_parameters(1, 48, 48)`
- Enable active scanning (type=1)
- Check device filter logic

**Connection fails:**
- Reduce security requirements
- Clear bonded devices: `bluetooth_clear_bonded_devices()`
- Check for conflicting connections

**Poor performance:**
- Reduce connection interval
- Disable other Bluetooth features
- Check for interference

**High power consumption:**
- Increase scan interval
- Use passive scanning
- Implement connection interval extension

### Debug Information

Enable comprehensive logging to diagnose issues:

```c
#define CONFIG_BLUEPAD32_LOG_LEVEL 2
#define ENABLE_LOG_DEBUG
#define ENABLE_LOG_VERBOSE
#define ENABLE_PRINTF_HEXDUMP
```

This will output detailed information about:
- Device discovery process
- Connection attempts
- Pairing procedures
- Data transmission
- Error conditions