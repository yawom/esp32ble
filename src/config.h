#ifndef CONFIG_H
#define CONFIG_H

#include <Arduino.h>
#include "boards/BoardProfiles.h"

// ============================================================================
// BUTTON CONFIGURATION
// ============================================================================

#define BUTTON_DEBOUNCE_MS      50
#define LONG_PRESS_DURATION_MS  3000  // 3 seconds for pairing mode / clear

// ============================================================================
// BLE CONFIGURATION
// ============================================================================

// Device name
#define BLE_DEVICE_NAME         "ESP32-BLE-DEMO"

// Service UUID (custom service)
#define SERVICE_UUID            "4fafc201-1fb5-459e-8fcc-c5c9c331914b"

// Characteristic UUIDs
#define COUNTER_CHAR_UUID       "beb5483e-36e1-4688-b7f5-ea07361b26a8"
#define PROXIMITY_CHAR_UUID     "cba1d466-344c-4be3-ab3f-189f80dd7518"
#define DEVICE_NAME_CHAR_UUID   "d8de624e-140f-4a22-8594-e2216b84a5f2"

// BLE advertising interval (milliseconds)
#define BLE_ADV_INTERVAL_MS     100

// Pairing mode timeout (milliseconds)
#define PAIRING_MODE_TIMEOUT_MS 60000  // 1 minute

// Maximum registered devices
#define MAX_REGISTERED_DEVICES  10

// ============================================================================
// STORAGE CONFIGURATION (using framework's LittleFSConfig)
// ============================================================================

// Config keys for storage
#define CONFIG_COUNTER_VALUE    "counter.value"
#define CONFIG_DEVICES_PREFIX   "devices"

// ============================================================================
// DISPLAY CONFIGURATION
// ============================================================================

// Display update interval (milliseconds)
#define DISPLAY_UPDATE_INTERVAL_MS  500

// ============================================================================
// SYSTEM STATE CONFIGURATION
// ============================================================================

enum class SystemState {
    INITIALIZING,
    NORMAL,
    PAIRING_MODE,
    ERROR
};

// ============================================================================
// DATA STRUCTURES
// ============================================================================

struct RegisteredDevice {
    uint8_t macAddress[6];
    uint32_t registeredTimestamp;
    bool isValid;

    RegisteredDevice() : registeredTimestamp(0), isValid(false) {
        memset(macAddress, 0, 6);
    }
};

#endif // CONFIG_H
