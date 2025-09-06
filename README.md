# Orinayo Pico - BLE MIDI Controller

A Raspberry Pi Pico W project that implements a BLE client using bluepad32 to connect to gaming controllers and convert their input to MIDI messages for music control.

## Features

### BLE Client Functionality
- **Automatic device discovery**: Scans for and discovers BLE HID devices (gamepads, keyboards, mice)
- **Seamless connection**: Automatically connects to supported BLE controllers
- **Pairing support**: Handles BLE pairing and bonding with security
- **Multi-device support**: Can connect to multiple BLE devices (configured for 1 device by default)
- **Device management**: Handles connection/disconnection events and automatic reconnection

### Supported BLE Devices
- Gaming controllers (Xbox, PlayStation, Switch Pro, etc.)
- BLE keyboards
- BLE mice
- Generic HID devices with gamepad appearance

### MIDI Output
- USB MIDI device functionality
- Real-time MIDI note generation from controller input
- Chord progression support
- Arranger keyboard control (Ketron devices)
- Guitar-style chord mapping

## BLE Client Implementation

The BLE client is implemented using the [bluepad32](https://github.com/ricardoquesada/bluepad32) library, which provides a robust Bluetooth Low Energy client stack for microcontrollers.

### Key Components

#### 1. BLE Scanning and Discovery (`pico_bluetooth.c`)
```c
// BLE scanning is automatically started after initialization
uni_bt_start_scanning_and_autoconnect_safe();
```

#### 2. Device Discovery Filtering
The system filters discovered devices based on their appearance and class of device:
```c
static uni_error_t pico_bluetooth_on_device_discovered(bd_addr_t addr, const char* name, uint16_t cod, uint8_t rssi) {
    // Filter out non-HID devices
    if (((cod & UNI_BT_COD_MINOR_MASK) & UNI_BT_COD_MINOR_KEYBOARD) == UNI_BT_COD_MINOR_KEYBOARD) {
        return UNI_ERROR_IGNORE_DEVICE;
    }
    return UNI_ERROR_SUCCESS;
}
```

#### 3. Connection Management
```c
static void pico_bluetooth_on_device_connected(uni_hid_device_t* d) {
    // Disable scanning when device connects to save power
    uni_bt_stop_scanning_safe();
}

static void pico_bluetooth_on_device_disconnected(uni_hid_device_t* d) {
    // Re-enable scanning when device disconnects
    uni_bt_start_scanning_and_autoconnect_safe();
}
```

#### 4. Data Processing
Controller input is processed in real-time:
```c
static void pico_bluetooth_on_controller_data(uni_hid_device_t* d, uni_controller_t* ctl) {
    // Process gamepad input and convert to MIDI
    // Handle button presses, analog sticks, triggers
}
```

## Configuration

### BLE Settings (`sdkconfig.h`)
```c
#define CONFIG_BLUEPAD32_ENABLE_BLE_BY_DEFAULT 1    // Enable BLE by default
#define CONFIG_BLUEPAD32_MAX_DEVICES 1              // Maximum connected devices
#define CONFIG_BLUEPAD32_GAP_SECURITY 1             // Enable security/pairing
```

### BTStack Configuration (`btstack_config.h`)
```c
#define ENABLE_LE_CENTRAL                           // Enable BLE central role
#define ENABLE_GATT_CLIENT_PAIRING                  // Enable GATT client with pairing
#define ENABLE_LE_SECURE_CONNECTIONS                // Enable secure connections
#define MAX_NR_HIDS_CLIENTS 4                       // Max HID service clients
#define MAX_NR_LE_DEVICE_DB_ENTRIES 16              // Max bonded devices
```

## Building

### Prerequisites
- Raspberry Pi Pico SDK
- CMake 3.12+
- GCC ARM toolchain

### Build Steps
```bash
mkdir build
cd build
cmake ..
make
```

### Flashing
1. Hold BOOTSEL button on Pico W
2. Connect to computer via USB
3. Copy `Orinayo.uf2` to the mounted drive

## Usage

### Initial Setup
1. Power on the Pico W
2. The device will automatically start BLE scanning (LED will be on)
3. Put your BLE controller in pairing mode
4. The device will automatically discover and connect (LED will turn off when connected)

### Controller Mapping
The current implementation maps gaming controller inputs to guitar-style chord progressions:

- **Face buttons**: Chord selection (Green, Red, Yellow, Blue, Orange)
- **D-pad left/right**: Strum down/up
- **D-pad up/down**: Transpose
- **Trigger buttons**: Style section control
- **Analog sticks**: Fill and break controls

### MIDI Output
The device appears as a USB MIDI device and outputs:
- Note On/Off messages for chord progressions
- System Exclusive messages for arranger control
- Real-time MIDI data

## LED Indicators

- **LED On (solid)**: Scanning for devices
- **LED Off**: Device connected and ready
- **LED Blinking**: During strum/chord actions

## Troubleshooting

### Device Won't Connect
1. Ensure device is in pairing mode
2. Check if device is already paired to another system
3. Reset bonding information (device will auto-delete keys on startup)

### No MIDI Output
1. Verify USB connection
2. Check MIDI device recognition in host software
3. Ensure controller is properly connected (LED should be off)

### Poor Performance
1. Move devices closer together
2. Avoid interference from other 2.4GHz devices
3. Ensure adequate power supply

## API Reference

### Main Functions

#### `bluetooth_init()`
Initializes the BLE client and starts scanning for devices.

#### `pico_bluetooth_on_controller_data()`
Processes incoming controller data and converts to MIDI output.

### BLE Client Functions (bluepad32)

#### `uni_bt_start_scanning_and_autoconnect_safe()`
Starts BLE scanning for compatible devices.

#### `uni_bt_stop_scanning_safe()`
Stops BLE scanning (typically when device connects).

#### `uni_hid_device_create()`
Creates a new HID device instance for discovered BLE device.

## Dependencies

- [bluepad32](https://github.com/ricardoquesada/bluepad32) - Bluetooth controller library
- [Raspberry Pi Pico SDK](https://github.com/raspberrypi/pico-sdk) - Hardware abstraction layer
- [BTStack](https://github.com/bluekitchen/btstack) - Bluetooth stack (included with bluepad32)
- [TinyUSB](https://github.com/hathach/tinyusb) - USB device stack

## License

This project is licensed under the GNU General Public License v3.0 - see the license headers in source files for details.

## Contributing

1. Fork the repository
2. Create a feature branch
3. Make your changes
4. Add tests if applicable
5. Submit a pull request

## Acknowledgments

- [bluepad32](https://github.com/ricardoquesada/bluepad32) by Ricardo Quesada for the excellent Bluetooth controller library
- Raspberry Pi Foundation for the Pico W platform
- BlueKitchen for the BTStack Bluetooth implementation