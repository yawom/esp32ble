#include "ble_app.h"

BLEApp::BLEApp()
    : ApplicationBase("ESP32-BLE-Demo", "1.0.0")
#if HAS_BUTTONS
    , button1(nullptr)
    , button2(nullptr)
#endif
    , currentState(SystemState::INITIALIZING)
    , lastDisplayUpdate(0) {
}

BLEApp::~BLEApp() {
#if HAS_BUTTONS
    if (button1) delete button1;
    if (button2) delete button2;
#endif
}

// ============================================================================
// ApplicationBase Lifecycle Hooks
// ============================================================================

const char* BLEApp::getWiFiAPName() {
    return "ESP32-BLE-SETUP";
}

void BLEApp::onSetup() {
    logger->log("BLEApp::onSetup() called");
    logger->log("Config pointer: %p", config);

    setupButtons();

    // Verify config is valid before proceeding
    if (!config) {
        logger->log("FATAL: Config is null - cannot initialize");
        currentState = SystemState::ERROR;
        return;
    }

    logger->log("Config is valid, initializing CounterApp");

    if (!CounterApp::getInstance().begin(config, &Logger::getInstance())) {
        logger->log("FATAL: Counter app initialization failed");
        currentState = SystemState::ERROR;
        return;
    }

    setupBLE();

    currentState = SystemState::NORMAL;
    logger->log("BLE app initialized successfully!");
}

void BLEApp::onStateUpdate(AppState state) {
    if (state == AppState::MQTT_CONNECT) {
        logger->log("Skipping MQTT - transitioning to RUNNING state");
        ApplicationBase::currentState = AppState::RUNNING;
    }
}

void BLEApp::onLoop() {
    BLEManager::getInstance().update();

    if (BLEManager::getInstance().isInPairingMode()) {
        currentState = SystemState::PAIRING_MODE;
    } else if (currentState == SystemState::PAIRING_MODE) {
        currentState = SystemState::NORMAL;
    }

    unsigned long now = millis();
    if (now - lastDisplayUpdate >= DISPLAY_UPDATE_INTERVAL_MS) {
        lastDisplayUpdate = now;
        updateDisplay();
    }
}

#if HAS_BUTTONS
void BLEApp::onAlwaysLoop() {
}
#endif

// ============================================================================
// Setup Helpers
// ============================================================================

void BLEApp::setupButtons() {
#if HAS_BUTTONS
    logger->log("Initializing buttons...");

    button1 = new ButtonHandler(PIN_BUTTON_1, true, true, LONG_PRESS_DURATION_MS, 250, BUTTON_DEBOUNCE_MS);
    button1->setOnClickCallback([this](int pinNumber, int clickCount) {
        if (BLEManager::getInstance().isInPairingMode()) {
            logger->log("Button 1 clicked - Exit pairing mode");
            BLEManager::getInstance().exitPairingMode();
            return;
        }

        logger->log("Button 1 clicked (count: %d)", clickCount);
        CounterApp::getInstance().decrement();
    });
    button1->setOnLongPressStartCallback([this](int pinNumber) {
        logger->log("Button 1 long press - Enter pairing mode");
        if (!BLEManager::getInstance().isInPairingMode()) {
            BLEManager::getInstance().enterPairingMode();
        }
    });

    button2 = new ButtonHandler(PIN_BUTTON_2, true, true, LONG_PRESS_DURATION_MS, 250, BUTTON_DEBOUNCE_MS);
    button2->setOnClickCallback([this](int pinNumber, int clickCount) {
        if (BLEManager::getInstance().isInPairingMode()) {
            logger->log("Button 2 clicked - Exit pairing mode");
            BLEManager::getInstance().exitPairingMode();
            return;
        }

        logger->log("Button 2 clicked (count: %d)", clickCount);
        CounterApp::getInstance().increment();
    });
    button2->setOnLongPressStartCallback([this](int pinNumber) {
        logger->log("Button 2 long press - Clear all registered devices");
        if (!BLEManager::getInstance().isInPairingMode()) {
            CounterApp::getInstance().clearAllDevices();
        }
    });

    logger->log("Buttons initialized successfully");
#else
    logger->log("No buttons available");
#endif
}

void BLEApp::setupBLE() {
    logger->log("Initializing BLE...");
    if (!BLEManager::getInstance().begin(&CounterApp::getInstance(), &Logger::getInstance())) {
        logger->log("FATAL: BLE initialization failed");
        currentState = SystemState::ERROR;
        return;
    }
}

// ============================================================================
// Display Update
// ============================================================================

void BLEApp::updateDisplay() {
#if HAS_DISPLAY
    LGFX& gfx = display->getGfx();
    gfx.fillScreen(TFT_BLACK);
    gfx.setTextColor(TFT_WHITE);

    switch (currentState) {
        case SystemState::INITIALIZING:
            gfx.setTextSize(2);
            gfx.setCursor(10, LCD_HEIGHT / 2 - 10);
            gfx.print("Initializing...");
            break;

        case SystemState::PAIRING_MODE:
            gfx.setTextSize(2);
            gfx.setTextColor(TFT_YELLOW);
            gfx.setCursor(10, 20);
            gfx.print("PAIRING MODE");

            gfx.setTextSize(1);
            gfx.setTextColor(TFT_WHITE);
            gfx.setCursor(10, 50);
            gfx.printf("Password: %s", BLEManager::getInstance().getPairingPassword());

            gfx.setCursor(10, 70);
            gfx.print("Waiting for device...");
            break;

        case SystemState::NORMAL:
            gfx.setTextSize(2);
            gfx.setCursor(10, 20);
            gfx.print("BLE DEMO");

            gfx.setTextSize(2);
            gfx.setCursor(10, 60);
            gfx.printf("Count: %d", CounterApp::getInstance().getValue());

            gfx.setTextSize(1);
            gfx.setCursor(10, 120);
            gfx.printf("Devices: %zu", CounterApp::getInstance().getRegisteredDeviceCount());

            gfx.setCursor(10, 140);
            if (CounterApp::getInstance().isConnectedDeviceNearby()) {
                gfx.setTextColor(TFT_GREEN);
                gfx.print("Device nearby!");
            } else {
                gfx.setTextColor(TFT_RED);
                gfx.print("No device");
            }

            gfx.setTextColor(TFT_WHITE);
            gfx.setCursor(10, 160);
            if (BLEManager::getInstance().isDeviceConnected()) {
                gfx.setTextColor(TFT_GREEN);
                gfx.print("BLE Connected");
            } else {
                gfx.setTextColor(TFT_CYAN);
                gfx.print("BLE Advertising");
            }
            break;

        case SystemState::ERROR:
            gfx.setTextSize(2);
            gfx.setTextColor(TFT_RED);
            gfx.setCursor(10, LCD_HEIGHT / 2 - 10);
            gfx.print("ERROR!");
            break;
    }
#endif
}
