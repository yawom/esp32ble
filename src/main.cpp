#include <Arduino.h>
#include "config.h"
#include "storage/storage_manager.h"
#include "ble/ble_manager.h"
#include "app/counter_app.h"
#include "ui/display_manager.h"
#include "input/button_handler.h"

// ============================================================================
// Application State
// ============================================================================

class Application : public ButtonHandlerCallbacks {
public:
    Application()
        : currentState(SystemState::INITIALIZING) {
    }

    void setup() {
        // Initialize serial for debugging
        Serial.begin(SERIAL_BAUD_RATE);
        delay(1000);

        DEBUG_PRINTLN("\n\n====================================");
        DEBUG_PRINTLN("ESP32 BLE Access Control Demo");
        DEBUG_PRINTLN("====================================\n");

        // Initialize display first (for visual feedback)
        if (!DisplayManager::getInstance().begin()) {
            DEBUG_PRINTLN("FATAL: Display initialization failed");
            currentState = SystemState::ERROR;
            return;
        }

        // Show initializing screen
        DisplayManager::getInstance().update(SystemState::INITIALIZING, 0, false);

        // Initialize storage
        if (!StorageManager::getInstance().begin()) {
            DEBUG_PRINTLN("FATAL: Storage initialization failed");
            currentState = SystemState::ERROR;
            return;
        }

        // Initialize counter app
        if (!CounterApp::getInstance().begin()) {
            DEBUG_PRINTLN("FATAL: Counter app initialization failed");
            currentState = SystemState::ERROR;
            return;
        }

        // Initialize BLE
        if (!BLEManager::getInstance().begin(&CounterApp::getInstance())) {
            DEBUG_PRINTLN("FATAL: BLE initialization failed");
            currentState = SystemState::ERROR;
            return;
        }

        // Initialize button handler
        if (!ButtonHandler::getInstance().begin(this)) {
            DEBUG_PRINTLN("FATAL: Button handler initialization failed");
            currentState = SystemState::ERROR;
            return;
        }

        // All systems initialized
        currentState = SystemState::NORMAL;
        DEBUG_PRINTLN("\nSystem initialized successfully!");
        DEBUG_PRINTLN("Ready for operation.\n");
    }

    void loop() {
        // Update button states
        ButtonHandler::getInstance().update();

        // Update BLE manager (for pairing mode timeout)
        BLEManager::getInstance().update();

        // Update current state based on BLE manager
        if (BLEManager::getInstance().isInPairingMode()) {
            currentState = SystemState::PAIRING_MODE;
        } else if (currentState == SystemState::PAIRING_MODE) {
            // Just exited pairing mode
            currentState = SystemState::NORMAL;
        }

        // Update display
        DisplayManager::getInstance().update(
            currentState,
            CounterApp::getInstance().getValue(),
            CounterApp::getInstance().isConnectedDeviceNearby(),
            BLEManager::getInstance().getPairingPassword(),
            CounterApp::getInstance().getRegisteredDeviceCount()
        );

        // Small delay to prevent tight loop
        delay(10);
    }

    // ButtonHandlerCallbacks implementation
    void onButton1Click() override {
        DEBUG_PRINTLN("APP: Button 1 click - Increment counter");
        CounterApp::getInstance().increment();
    }

    void onButton2Click() override {
        DEBUG_PRINTLN("APP: Button 2 click - Decrement counter");
        CounterApp::getInstance().decrement();
    }

    void onButton1LongPress() override {
        DEBUG_PRINTLN("APP: Button 1 long press - Enter pairing mode");

        if (!BLEManager::getInstance().isInPairingMode()) {
            BLEManager::getInstance().enterPairingMode();
        }
    }

    void onButton2LongPress() override {
        DEBUG_PRINTLN("APP: Button 2 long press - Clear all registered devices");

        // Only allow clearing if not in pairing mode
        if (!BLEManager::getInstance().isInPairingMode()) {
            CounterApp::getInstance().clearAllDevices();
            DEBUG_PRINTLN("All registered devices cleared");
        }
    }

private:
    SystemState currentState;
};

// ============================================================================
// Global Application Instance
// ============================================================================

Application app;

// ============================================================================
// Arduino Entry Points
// ============================================================================

void setup() {
    app.setup();
}

void loop() {
    app.loop();
}
