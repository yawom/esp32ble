#ifndef CONFIG_H
#define CONFIG_H

#include <Arduino.h>

// ============================================================================
// HARDWARE CONFIGURATION - Lilygo T-Display S3
// ============================================================================

// Display pins (for TFT_eSPI library)
#define TFT_WIDTH       170
#define TFT_HEIGHT      320
#define TFT_ROTATION    1  // Landscape mode

// Button pins
#define BUTTON_1_PIN    0   // Left button - Increment/Pairing mode
#define BUTTON_2_PIN    14  // Right button - Decrement/Clear devices

// Future gate relay control
#define GATE_RELAY_PIN  21  // GPIO for gate control (not used in initial demo)

// ============================================================================
// BUTTON CONFIGURATION
// ============================================================================

#define BUTTON_DEBOUNCE_MS      50
#define LONG_PRESS_DURATION_MS  5000  // 5 seconds for pairing mode / clear

// ============================================================================
// BLE CONFIGURATION
// ============================================================================

// Device name
#define BLE_DEVICE_NAME         "ESP32-Access"

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
// STORAGE CONFIGURATION
// ============================================================================

// LittleFS mount point
#define FS_MOUNT_POINT          "/littlefs"

// File paths (LittleFS automatically adds mount point)
#define DEVICES_FILE_PATH       "/devices.dat"
#define COUNTER_FILE_PATH       "/counter.dat"

// Storage format version (for future compatibility)
#define STORAGE_FORMAT_VERSION  1

// ============================================================================
// DISPLAY CONFIGURATION
// ============================================================================

// Display update interval (milliseconds)
#define DISPLAY_UPDATE_INTERVAL_MS  500

// UI Colors (RGB565 format)
#define COLOR_BACKGROUND        0x0000  // Black
#define COLOR_TEXT_PRIMARY      0xFFFF  // White
#define COLOR_TEXT_SECONDARY    0x7BEF  // Light gray
#define COLOR_ACCENT            0x07FF  // Cyan
#define COLOR_WARNING           0xFD20  // Orange
#define COLOR_ERROR             0xF800  // Red
#define COLOR_SUCCESS           0x07E0  // Green

// Font sizes
#define FONT_SIZE_SMALL         1
#define FONT_SIZE_MEDIUM        2
#define FONT_SIZE_LARGE         3
#define FONT_SIZE_XLARGE        4

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

// ============================================================================
// DEBUG CONFIGURATION
// ============================================================================

#define SERIAL_BAUD_RATE        115200
#define ENABLE_DEBUG_LOGGING    true

#if ENABLE_DEBUG_LOGGING
    #define DEBUG_PRINT(x)      Serial.print(x)
    #define DEBUG_PRINTLN(x)    Serial.println(x)
    #define DEBUG_PRINTF(...)   Serial.printf(__VA_ARGS__)
#else
    #define DEBUG_PRINT(x)
    #define DEBUG_PRINTLN(x)
    #define DEBUG_PRINTF(...)
#endif

#endif // CONFIG_H
