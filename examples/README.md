# Examples

This directory contains example code demonstrating how to use the BLE client functionality with bluepad32.

## BLE Client Example

### File: `ble_client_example.c`

A standalone example that demonstrates how to create a basic BLE client using bluepad32. This example shows:

- **Device Discovery**: Scans for BLE HID devices and filters them by type
- **Connection Management**: Automatically connects to discovered devices
- **Data Processing**: Receives and processes input from connected devices
- **State Management**: Handles connection/disconnection events

### Building the Example

From the examples directory:
```bash
mkdir build
cd build
cmake ..
make
```

This will create `ble_client_example.uf2` which can be flashed to the Pico W.

### Running the Example

1. Flash `ble_client_example.uf2` to your Pico W
2. Connect a UART adapter to pins GP0 (TX) and GP1 (RX) for debug output
3. Open a serial terminal at 115200 baud
4. Put a BLE device (gamepad, keyboard, or mouse) in pairing mode
5. Watch the debug output as devices are discovered and connected

### Expected Output

```
BLE Client Example: Hardware initialized
BLE Client Example: Starting initialization...
BLE Client: Custom platform registered
BLE Client: bluepad32 initialized
BLE Client: Bluetooth initialization complete
BLE Client: Started scanning for devices...
BLE Client: Found device: Xbox Wireless Controller (XX:XX:XX:XX:XX:XX)
  - Class of Device: 0x2508
  - RSSI: -45 dBm
  - Type: Gamepad - ACCEPTING
BLE Client: Device connected: Xbox Wireless Controller (XX:XX:XX:XX:XX:XX)
BLE Client: Stopped scanning (device connected)
BLE Client: Device ready: Xbox Wireless Controller
Gamepad [Xbox Wireless Controller]: buttons=0x0001, dpad=0x00, axis_x=0, axis_y=0
```

### LED Behavior

- **LED ON**: Scanning for devices
- **LED OFF**: Device connected and ready
- **LED FLASHING**: Button activity detected

### Differences from Main Application

This example differs from the main MIDI application in several ways:

1. **Output**: Uses UART for debug output instead of USB MIDI
2. **Simplified**: Only demonstrates basic BLE client functionality
3. **Educational**: Heavily commented to explain each step
4. **Standalone**: Self-contained example that can be studied independently

### Extending the Example

You can extend this example to:

- Add support for multiple simultaneous connections
- Implement custom data processing for specific device types
- Add your own application logic in the main loop
- Integrate with other hardware (sensors, displays, etc.)
- Create a custom HID device that forwards input to other systems

### Troubleshooting

**No debug output**: Ensure UART connection is correct (GP0=TX, GP1=RX, common ground)

**Device not found**: 
- Ensure device is in pairing mode
- Check if device supports BLE (not just classic Bluetooth)
- Try moving devices closer together

**Connection fails**: 
- Some devices may require specific pairing procedures
- Check debug output for error messages
- Try resetting both devices