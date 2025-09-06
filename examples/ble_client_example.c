/**
 * BLE Client Example - Simple HID Device Scanner
 * 
 * This example demonstrates how to use bluepad32 to create a basic BLE client
 * that scans for and connects to BLE HID devices (keyboards, mice, gamepads).
 * 
 * Features demonstrated:
 * - BLE device discovery and filtering
 * - Automatic connection to discovered devices
 * - Basic input processing
 * - Connection state management
 * 
 * Copyright 2024 Samyar Sadat Akhavi
 * Licensed under GNU GPL v3.0
 */

#include <stdio.h>
#include "pico/cyw43_arch.h"
#include "pico/stdlib.h"
#include "hardware/gpio.h"

// bluepad32 includes
#include <bt/uni_bt.h>
#include <btstack.h>
#include <controller/uni_gamepad.h>
#include <uni.h>
#include <uni_hid_device.h>

// Configuration
#define MAX_DEVICE_NAME_LEN 64
#define LED_PIN CYW43_WL_GPIO_LED_PIN

// Device state tracking
static bool scanning_active = false;
static int connected_devices = 0;

/**
 * Platform initialization - called once at startup
 */
static void ble_client_init(int argc, const char** argv) {
    printf("BLE Client Example: Initializing...\n");
    ARG_UNUSED(argc);
    ARG_UNUSED(argv);
}

/**
 * Called when Bluetooth initialization is complete
 * This is where we start scanning for devices
 */
static void ble_client_on_init_complete(void) {
    printf("BLE Client: Bluetooth initialization complete\n");
    
    // Optional: Delete stored bonding keys for fresh scanning
    uni_bt_del_keys_unsafe();
    
    // Start scanning for BLE devices
    uni_bt_start_scanning_and_autoconnect_safe();
    scanning_active = true;
    
    // Turn on LED to indicate scanning
    cyw43_arch_gpio_put(LED_PIN, true);
    
    printf("BLE Client: Started scanning for devices...\n");
}

/**
 * Device discovery filter - called for each discovered device
 * Return UNI_ERROR_SUCCESS to allow connection, or UNI_ERROR_IGNORE_DEVICE to ignore
 */
static uni_error_t ble_client_on_device_discovered(bd_addr_t addr, const char* name, uint16_t cod, uint8_t rssi) {
    const char* device_name = (name && strlen(name) > 0) ? name : "Unknown";
    
    printf("BLE Client: Found device: %s (%02X:%02X:%02X:%02X:%02X:%02X)\n", 
           device_name, addr[0], addr[1], addr[2], addr[3], addr[4], addr[5]);
    printf("  - Class of Device: 0x%04X\n", cod);
    printf("  - RSSI: %d dBm\n", rssi);
    
    // Filter devices by Class of Device (CoD)
    // Accept gamepads, keyboards, mice, and joysticks
    uint16_t major_class = cod & UNI_BT_COD_MAJOR_MASK;
    uint16_t minor_class = cod & UNI_BT_COD_MINOR_MASK;
    
    if (major_class == UNI_BT_COD_MAJOR_PERIPHERAL) {
        switch (minor_class) {
            case UNI_BT_COD_MINOR_GAMEPAD:
                printf("  - Type: Gamepad - ACCEPTING\n");
                return UNI_ERROR_SUCCESS;
                
            case UNI_BT_COD_MINOR_JOYSTICK:
                printf("  - Type: Joystick - ACCEPTING\n");
                return UNI_ERROR_SUCCESS;
                
            case UNI_BT_COD_MINOR_KEYBOARD:
                printf("  - Type: Keyboard - ACCEPTING\n");
                return UNI_ERROR_SUCCESS;
                
            case UNI_BT_COD_MINOR_MICE:
                printf("  - Type: Mouse - ACCEPTING\n");
                return UNI_ERROR_SUCCESS;
                
            default:
                printf("  - Type: Other peripheral (0x%04X) - IGNORING\n", minor_class);
                return UNI_ERROR_IGNORE_DEVICE;
        }
    }
    
    printf("  - Type: Non-peripheral device - IGNORING\n");
    return UNI_ERROR_IGNORE_DEVICE;
}

/**
 * Called when a device successfully connects
 */
static void ble_client_on_device_connected(uni_hid_device_t* d) {
    printf("BLE Client: Device connected: %s (%02X:%02X:%02X:%02X:%02X:%02X)\n", 
           d->name, d->conn.btaddr[0], d->conn.btaddr[1], d->conn.btaddr[2], 
           d->conn.btaddr[3], d->conn.btaddr[4], d->conn.btaddr[5]);
    
    connected_devices++;
    
    // Stop scanning when we have a device connected (saves power)
    if (scanning_active) {
        uni_bt_stop_scanning_safe();
        scanning_active = false;
        printf("BLE Client: Stopped scanning (device connected)\n");
    }
}

/**
 * Called when a device disconnects
 */
static void ble_client_on_device_disconnected(uni_hid_device_t* d) {
    printf("BLE Client: Device disconnected: %s (%02X:%02X:%02X:%02X:%02X:%02X)\n", 
           d->name, d->conn.btaddr[0], d->conn.btaddr[1], d->conn.btaddr[2], 
           d->conn.btaddr[3], d->conn.btaddr[4], d->conn.btaddr[5]);
    
    connected_devices--;
    
    // Restart scanning when device disconnects
    if (!scanning_active) {
        uni_bt_start_scanning_and_autoconnect_safe();
        scanning_active = true;
        cyw43_arch_gpio_put(LED_PIN, true);
        printf("BLE Client: Restarted scanning (device disconnected)\n");
    }
}

/**
 * Called when a device is ready for communication
 */
static uni_error_t ble_client_on_device_ready(uni_hid_device_t* d) {
    printf("BLE Client: Device ready: %s\n", d->name);
    
    // Turn off LED to indicate successful connection
    cyw43_arch_gpio_put(LED_PIN, false);
    
    return UNI_ERROR_SUCCESS;
}

/**
 * Property getter (not used in this example)
 */
static const uni_property_t* ble_client_get_property(uni_property_idx_t idx) {
    ARG_UNUSED(idx);
    return NULL;
}

/**
 * Main controller data handler - called when input data is received
 */
static void ble_client_on_controller_data(uni_hid_device_t* d, uni_controller_t* ctl) {
    static int report_counter = 0;
    
    // Print device data every 100 reports to avoid spam
    if (++report_counter >= 100) {
        report_counter = 0;
        
        switch (ctl->klass) {
            case UNI_CONTROLLER_CLASS_GAMEPAD:
                printf("Gamepad [%s]: buttons=0x%04X, dpad=0x%02X, axis_x=%d, axis_y=%d\n",
                       d->name, ctl->gamepad.buttons, ctl->gamepad.dpad, 
                       ctl->gamepad.axis_x, ctl->gamepad.axis_y);
                
                // Example: Flash LED on button press
                if (ctl->gamepad.buttons != 0) {
                    cyw43_arch_gpio_put(LED_PIN, true);
                } else {
                    cyw43_arch_gpio_put(LED_PIN, false);
                }
                break;
                
            case UNI_CONTROLLER_CLASS_MOUSE:
                printf("Mouse [%s]: buttons=0x%02X, delta_x=%d, delta_y=%d\n",
                       d->name, ctl->mouse.buttons, ctl->mouse.delta_x, ctl->mouse.delta_y);
                break;
                
            case UNI_CONTROLLER_CLASS_KEYBOARD:
                printf("Keyboard [%s]: pressed_keys_count=%d\n",
                       d->name, ctl->keyboard.pressed_keys_count);
                break;
                
            default:
                printf("Unknown device class: %d\n", ctl->klass);
                break;
        }
    }
}

/**
 * Out-of-band event handler (optional)
 */
static void ble_client_on_oob_event(uni_platform_oob_event_t event, void* data) {
    switch (event) {
        case UNI_PLATFORM_OOB_BLUETOOTH_ENABLED:
            printf("BLE Client: Bluetooth enabled: %s\n", (bool)(data) ? "true" : "false");
            break;
            
        case UNI_PLATFORM_OOB_GAMEPAD_SYSTEM_BUTTON:
            printf("BLE Client: System button pressed\n");
            break;
            
        default:
            printf("BLE Client: Unknown OOB event: 0x%04x\n", event);
            break;
    }
}

/**
 * Platform structure definition
 * This defines the callbacks that bluepad32 will call
 */
static struct uni_platform ble_client_platform = {
    .name = "BLE Client Example",
    .init = ble_client_init,
    .on_init_complete = ble_client_on_init_complete,
    .on_device_discovered = ble_client_on_device_discovered,
    .on_device_connected = ble_client_on_device_connected,
    .on_device_disconnected = ble_client_on_device_disconnected,
    .on_device_ready = ble_client_on_device_ready,
    .on_oob_event = ble_client_on_oob_event,
    .on_controller_data = ble_client_on_controller_data,
    .get_property = ble_client_get_property,
};

/**
 * Initialize the BLE client
 */
void ble_client_example_init(void) {
    printf("BLE Client Example: Starting initialization...\n");
    
    // Disable Wi-Fi modes (we only need Bluetooth)
    cyw43_arch_disable_ap_mode();
    
    // Register our custom platform with bluepad32
    uni_platform_set_custom(&ble_client_platform);
    printf("BLE Client: Custom platform registered\n");
    
    // Initialize bluepad32
    uni_init(0, NULL);
    printf("BLE Client: bluepad32 initialized\n");
}

/**
 * Main function for the example
 */
int main() {
    // Initialize Pico W
    if (cyw43_arch_init() != PICO_OK) {
        printf("Failed to initialize cyw43\n");
        return 1;
    }
    
    printf("BLE Client Example: Hardware initialized\n");
    
    // Initialize the BLE client
    ble_client_example_init();
    
    printf("BLE Client Example: Running... (LED indicates scanning/connection status)\n");
    printf("- LED ON: Scanning for devices\n");
    printf("- LED OFF: Device connected and ready\n");
    printf("- LED FLASH: Activity (button presses, etc.)\n");
    
    // Main loop - bluepad32 handles everything in the background
    while (true) {
        // Small delay to prevent busy waiting
        sleep_ms(10);
        
        // You could add other application logic here
    }
    
    return 0;
}