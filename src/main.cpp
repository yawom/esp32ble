#include <Arduino.h>
#include "app/ble_app.h"
#include "logger/Logger.h"
#include "logger/SerialSink.h"

SerialSink serialSink;

// ============================================================================
// Global Application Instance
// ============================================================================

BLEApp *app = nullptr;

// ============================================================================
// Arduino Entry Points
// ============================================================================

void setup() {
    Serial.begin(115200);

    logger = new Logger();
    logger->addSink(&serialSink);

    app = new BLEApp(logger);

    app->setup();
}

void loop() {
    app->run();
}
