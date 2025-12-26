#ifndef BLE_APP_H
#define BLE_APP_H

#include <Arduino.h>
#include "config/IConfig.h"
#include "config/LittleFSConfig.h"
#include "logger/Logger.h"
#include "logger/SerialSink.h"
#include "boards/BoardProfiles.h"

#if HAS_DISPLAY
#include "display/Display.h"
#endif

#if HAS_BUTTONS
#include "input/ButtonHandler.h"
#endif

#include "../config.h"
#include "../ble/ble_manager.h"
#include "counter_app.h"

/**
 * BLE Application for ESP32
 * Simplified app without WiFi/MQTT, focused on BLE
 */
class BLEApp {
public:
    BLEApp(Logger* logger);
    virtual ~BLEApp();

    /**
     * Setup the application - call this from Arduino setup()
     */
    void setup();

    /**
     * Main application loop - call this from Arduino loop()
     */
    void run();

private:
    // Core components
    Logger* logger;
    SerialSink serialSink;
    IConfig* config;

#if HAS_DISPLAY
    Display* display;
#endif

#if HAS_BUTTONS
    ButtonHandler* button1;
    ButtonHandler* button2;
#endif

    // Application state
    SystemState currentState;
    unsigned long lastDisplayUpdate;

    // Setup helpers
    void setupLogger();
    void setupConfig();
    void setupDisplay();
    void setupButtons();
    void setupBLE();

    // Display update
    void updateDisplay();
};

#endif // BLE_APP_H
