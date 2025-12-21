# ESP32 BLE Access Control Demo

A comprehensive ESP32 BLE (Bluetooth Low Energy) access control demonstration project for the Lilygo T-Display S3 board. This project showcases iPhone proximity detection via BLE, device registration, and a counter app with persistent storage.

## Features

- **BLE Peripheral Mode**: ESP32 acts as a BLE peripheral device that iPhones can connect to
- **Proximity Detection**: Detects when a registered iPhone is nearby and displays status
- **Demo Counter App**: Simple counter that can be incremented/decremented via buttons or BLE Scanner app
- **Device Registration**: Secure pairing mode with randomly generated password
- **Persistent Storage**: Uses LittleFS to store counter value and registered devices
- **Visual Feedback**: Full-color TFT display shows system status, counter value, and pairing information
- **Button Controls**: Two buttons for counter control and system management

## Hardware Requirements

- **Lilygo T-Display S3** - ESP32-S3 development board with built-in 170x320 TFT display
- **Two built-in buttons**:
  - Button 1 (GPIO 0): Increment counter / Enter pairing mode (5s hold)
  - Button 2 (GPIO 14): Decrement counter / Clear registered devices (5s hold)

## Software Architecture

The project uses a modular architecture with clear separation of concerns:

```
src/
├── main.cpp                    # Application orchestration and state machine
├── config.h                    # Compile-time constants and configuration
├── ble/
│   ├── ble_manager.h/cpp      # BLE peripheral, GATT services, pairing mode
├── storage/
│   ├── storage_manager.h/cpp  # LittleFS file operations
├── app/
│   ├── counter_app.h/cpp      # Counter logic and BLE callbacks
├── ui/
│   ├── display_manager.h/cpp  # TFT display rendering
└── input/
    ├── button_handler.h/cpp   # Button input with ESP32Button library
```

### Module Responsibilities

- **config.h**: All compile-time constants (pins, UUIDs, timeouts, colors)
- **ble_manager**: BLE stack initialization, advertising, GATT structure, pairing mode
- **storage_manager**: LittleFS operations for persistent data
- **counter_app**: Counter state, device registration, BLE event handling
- **display_manager**: All screen rendering and UI updates
- **button_handler**: Button debouncing, click/long-press detection
- **main.cpp**: System initialization and main loop

## BLE Service Structure

**Service UUID**: `4fafc201-1fb5-459e-8fcc-c5c9c331914b`

**Characteristics**:
- **Counter** (`beb5483e-36e1-4688-b7f5-ea07361b26a8`): Read/Write/Notify - 32-bit integer counter value
- **Proximity** (`cba1d466-344c-4be3-ab3f-189f80dd7518`): Read/Notify - Boolean proximity status
- **Device Name** (`d8de624e-140f-4a22-8594-e2216b84a5f2`): Read - Device name string

## Usage

### Normal Operation

1. Power on the device
2. Display shows current counter value and system status
3. Click Button 1 to increment counter
4. Click Button 2 to decrement counter
5. Counter value is automatically saved to flash storage

### Pairing a New Device

1. Hold Button 1 for 5 seconds to enter pairing mode
2. A 6-digit password appears on the screen
3. On your iPhone, open a BLE Scanner app (e.g., "nRF Connect", "LightBlue")
4. Scan for "ESP32-Access" device
5. Connect to the device
6. Your iPhone is now registered automatically
7. Pairing mode exits after 60 seconds or when you reconnect

### BLE Scanner Operations

When connected via a BLE Scanner app:
- **Read Counter**: Read the Counter characteristic to get current value
- **Write Counter**: Write a 32-bit integer to set a new counter value
- **Subscribe to Updates**: Enable notifications to receive real-time counter changes

### Clearing Registered Devices

1. Hold Button 2 for 5 seconds
2. All registered devices are cleared from memory and storage
3. Display shows "Reg: 0/10"

## Building and Uploading

### Prerequisites

- [PlatformIO](https://platformio.org/) installed (VS Code extension or CLI)
- USB cable to connect T-Display S3 to your computer

### Build Commands

```bash
# Build the project
pio run

# Upload to device
pio run --target upload

# Open serial monitor
pio device monitor

# Build, upload, and monitor in one command
pio run --target upload && pio device monitor
```

## Configuration

All configuration is in `src/config.h`:

- **Hardware pins**: Button and display GPIO assignments
- **BLE settings**: Device name, service/characteristic UUIDs, advertising interval
- **Timing**: Long-press duration, pairing timeout, display update interval
- **Storage paths**: LittleFS file paths
- **UI colors**: RGB565 color definitions
- **Debug logging**: Enable/disable serial debug output

## Future Enhancements

- **Gate Relay Control**: Add GPIO output to control a gate or door relay
- **Authorization Check**: Only allow registered devices to control the gate
- **Multiple Users**: Support up to 10 registered devices
- **Activity Log**: Store connection history in flash
- **Low Power Mode**: Sleep when no activity detected
- **WiFi Integration**: Remote monitoring and control via WiFi

## Dependencies

This project uses the following libraries (automatically managed by PlatformIO):

- **TFT_eSPI**: Display driver for T-Display S3
- **ESP32Button**: Button handling with debouncing
- **Arduino ESP32**: ESP32 Arduino framework with BLE support
- **LittleFS**: Built-in file system for ESP32

## Troubleshooting

### Display Not Working

- Check TFT_eSPI build flags in `platformio.ini`
- Verify display pins match T-Display S3 hardware

### BLE Connection Issues

- Ensure Bluetooth is enabled on your iPhone
- Try power cycling the ESP32
- Check serial monitor for connection logs
- Some BLE scanner apps work better than others (try "nRF Connect")

### Buttons Not Responding

- Check button GPIO pins in `config.h`
- Verify debounce settings (default 50ms)
- Hold long-press for full 5 seconds

### Storage Errors

- Format flash: Power cycle device twice quickly
- Check available flash space (monitor serial output)

## License

MIT License - See LICENSE file for details

## Author

Created with Claude Code for ESP32 BLE demonstration and learning purposes.
