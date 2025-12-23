#include <Arduino.h>
#include "config.h"
#include "storage/storage_manager.h"
#include "ble/ble_manager.h"
#include "app/counter_app.h"
#include "ui/display_manager.h"
#include <ESP32ButtonHandler.h>

// ============================================================================
// Application State
// ============================================================================

class Application {
public:
    Application()
        : currentState(SystemState::INITIALIZING)
        , button1(nullptr)
        , button2(nullptr) {
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

        // Initialize buttons
        DEBUG_PRINTLN("Initializing buttons...");

        button1 = new ESP32ButtonHandler(
            BUTTON_1_PIN,
            true,                          // activeLow
            true,                          // pullUp
            LONG_PRESS_DURATION_MS,        // holdThreshold
            250,                           // multiClickThreshold
            BUTTON_DEBOUNCE_MS             // debounceDelay
        );

        button2 = new ESP32ButtonHandler(
            BUTTON_2_PIN,
            true,
            true,
            LONG_PRESS_DURATION_MS,
            250,
            BUTTON_DEBOUNCE_MS
        );

        if (!button1 || !button2) {
            DEBUG_PRINTLN("FATAL: Button initialization failed");
            currentState = SystemState::ERROR;
            return;
        }

        // Setup button callbacks
        button1->setOnClickCallback([this](ESP32ButtonHandler* handler, int clickCount) {
            DEBUG_PRINTF("Button 1 clicked (count: %d)\n", clickCount);
            CounterApp::getInstance().increment();
        });

        button1->setOnLongPressStartCallback([this](ESP32ButtonHandler* handler) {
            DEBUG_PRINTLN("Button 1 long press - Enter pairing mode");
            if (!BLEManager::getInstance().isInPairingMode()) {
                BLEManager::getInstance().enterPairingMode();
            }
        });

        button2->setOnClickCallback([this](ESP32ButtonHandler* handler, int clickCount) {
            DEBUG_PRINTF("Button 2 clicked (count: %d)\n", clickCount);
            CounterApp::getInstance().decrement();
        });

        button2->setOnLongPressStartCallback([this](ESP32ButtonHandler* handler) {
            DEBUG_PRINTLN("Button 2 long press - Clear all registered devices");
            if (!BLEManager::getInstance().isInPairingMode()) {
                CounterApp::getInstance().clearAllDevices();
                DEBUG_PRINTLN("All registered devices cleared");
            }
        });

        DEBUG_PRINTLN("Buttons initialized successfully");

        // All systems initialized
        currentState = SystemState::NORMAL;
        DEBUG_PRINTLN("\nSystem initialized successfully!");
        DEBUG_PRINTLN("Ready for operation.\n");
    }

    ~Application() {
        if (button1) delete button1;
        if (button2) delete button2;
    }

    void loop() {
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
            CounterApp::getInstance().getRegisteredDeviceCount(),
            BLEManager::getInstance().isDeviceConnected()
        );

        // Small delay to prevent tight loop
        delay(10);
    }

private:
    SystemState currentState;
    ESP32ButtonHandler* button1;
    ESP32ButtonHandler* button2;
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
