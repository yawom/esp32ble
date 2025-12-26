#include <Arduino.h>
#include "app/ble_app.h"
#include "logger/Logger.h"
#include "logger/SerialSink.h"

// Global logger instance and serial sink
// Logger *logger = new Logger();
SerialSink serialSink;

// ============================================================================
// Global Application Instance
// ============================================================================

BLEApp *app = nullptr;

// ============================================================================
// Arduino Entry Points
// ============================================================================

void setup() {
    // Initialize serial port for logging
    Serial.begin(115200);

    // Add serial sink to logger
    logger = new Logger();
    logger->addSink(&serialSink);

    app = new BLEApp(logger);

    app->setup();
}

void loop() {
    app->run();
}
