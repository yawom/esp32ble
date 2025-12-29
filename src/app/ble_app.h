#ifndef BLE_APP_H
#define BLE_APP_H

#include "app/ApplicationBase.h"

#if HAS_BUTTONS
#include "input/ButtonHandler.h"
#endif

#include "../config.h"
#include "../ble/ble_manager.h"
#include "counter_app.h"

/**
 * BLE Application for ESP32
 * Inherits from ApplicationBase for WiFi, MQTT, and time sync infrastructure
 */
class BLEApp : public ApplicationBase {
public:
    BLEApp();
    virtual ~BLEApp();

protected:
    // ApplicationBase lifecycle hooks (override)
    void onSetup() override;
    void onLoop() override;
    void onStateUpdate(AppState currentState) override;
    const char* getWiFiAPName() override;

#if HAS_BUTTONS
    void onAlwaysLoop() override;
#endif

private:
#if HAS_BUTTONS
    ButtonHandler* button1;
    ButtonHandler* button2;
#endif

    // Application state
    SystemState currentState;
    unsigned long lastDisplayUpdate;

    // Setup helpers
    void setupButtons();
    void setupBLE();

    // Display update
    void updateDisplay();
};

#endif // BLE_APP_H
