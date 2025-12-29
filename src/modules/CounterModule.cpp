#include "boards/BoardProfiles.h"

#if HAS_DISPLAY

#include <Arduino.h>
#include "CounterModule.h"
#include "logger/Logger.h"

CounterModule::CounterModule(Logger* logger)
    : Module("CounterModule", logger, 4096, 2), lastUpdateTime(0) {
}

CounterModule::~CounterModule() {
}

void CounterModule::setup() {
    _logger->log("CounterModule: Module setup");

    if (region) {
        region->clear(TFT_BLACK);
        draw();
        region->refresh();
    }

    lastUpdateTime = millis();
}

void CounterModule::loop() {
    if (!region) {
        return;
    }

    unsigned long currentTime = millis();
    if (currentTime - lastUpdateTime > 500) {
        draw();
        lastUpdateTime = currentTime;
        region->refresh();
    }
}

void CounterModule::handleEvent(const ModuleEvent& event) {
    switch (event.type) {
        case ModuleEventType::TIMER_TICK:
            draw();
            break;
        default:
            break;
    }
}

void CounterModule::draw() {
    if (!region) return;

    region->clear(TFT_BLACK);

    int16_t x = 10;
    int16_t y = 10;
    const int16_t lineHeight = 20;

    if (BLEManager::getInstance().isInPairingMode()) {
        region->setTextSize(2);
        region->setTextColor(TFT_YELLOW);
        region->setCursor(x, y);
        region->print("PAIRING MODE");

        y += 30;
        region->setTextSize(1);
        region->setTextColor(TFT_WHITE);
        region->setCursor(x, y);
        region->printf("Password: %s", BLEManager::getInstance().getPairingPassword());

        y += lineHeight;
        region->setCursor(x, y);
        region->print("Waiting for device...");
    } else {
        y += 10;
        region->setTextSize(2);
        region->setTextColor(TFT_WHITE);
        region->setCursor(x, y);
        region->printf("Count: %d", CounterApp::getInstance().getValue());

        y += 40;
        region->setTextSize(1);
        region->setCursor(x, y);
        region->printf("Devices: %zu", CounterApp::getInstance().getRegisteredDeviceCount());

        y += lineHeight;
        region->setCursor(x, y);
        if (CounterApp::getInstance().isConnectedDeviceNearby()) {
            region->setTextColor(TFT_GREEN);
            region->print("Device nearby!");
        } else {
            region->setTextColor(TFT_RED);
            region->print("No device");
        }

        y += lineHeight;
        region->setTextColor(TFT_WHITE);
        region->setCursor(x, y);
        if (BLEManager::getInstance().isDeviceConnected()) {
            region->setTextColor(TFT_GREEN);
            region->print("BLE Connected");
        } else {
            region->setTextColor(TFT_CYAN);
            region->print("BLE Advertising");
        }
    }

    region->refresh();
}

#endif
