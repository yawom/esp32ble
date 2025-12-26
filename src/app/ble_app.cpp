#include "ble_app.h"

BLEApp::BLEApp(Logger *logger)
    : logger(logger)
    , config(nullptr)
#if HAS_DISPLAY
    , display(nullptr)
#endif
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
#if HAS_DISPLAY
    if (display) delete display;
#endif
    if (config) delete config;
    if (logger) delete logger;
}

void BLEApp::setup() {
    setupLogger();
    setupConfig();
    setupDisplay();
    setupButtons();

    // Initialize counter app
    if (!CounterApp::getInstance().begin(config)) {
        logger->log("FATAL: Counter app initialization failed");
        currentState = SystemState::ERROR;
        return;
    }

    setupBLE();

    // All systems initialized
    currentState = SystemState::NORMAL;
    logger->log("System initialized successfully!");
}

void BLEApp::run() {
    // Update BLE manager (for pairing mode timeout)
    BLEManager::getInstance().update();

    // Update current state based on BLE manager
    if (BLEManager::getInstance().isInPairingMode()) {
        currentState = SystemState::PAIRING_MODE;
    } else if (currentState == SystemState::PAIRING_MODE) {
        // Just exited pairing mode
        currentState = SystemState::NORMAL;
    }

    // Update display periodically
    unsigned long now = millis();
    if (now - lastDisplayUpdate >= DISPLAY_UPDATE_INTERVAL_MS) {
        lastDisplayUpdate = now;
        updateDisplay();
    }

    // Small delay to prevent tight loop
    delay(10);
}

void BLEApp::setupLogger() {
    // Serial sink is already added in main.cpp setup()
    logger->log("\n\n====================================");
    logger->log("ESP32 BLE Access Control Demo");
    logger->log("====================================\n");
}

void BLEApp::setupConfig() {
    logger->log("Initializing configuration...");
    config = new LittleFSConfig(logger);
    config->load();
}

void BLEApp::setupDisplay() {
#if HAS_DISPLAY
    logger->log("Initializing display...");
    display = new Display();
    if (!display->setup()) {
        logger->log("FATAL: Display initialization failed");
        currentState = SystemState::ERROR;
        return;
    }

    // Show initializing screen
    display->clearDisplay(TFT_BLACK);
    display->getGfx().setTextColor(TFT_WHITE);
    display->getGfx().setTextSize(2);
    display->getGfx().setCursor(10, LCD_HEIGHT / 2 - 10);
    display->getGfx().print("Initializing...");
#else
    logger->log("No display available");
#endif
}

void BLEApp::setupButtons() {
#if HAS_BUTTONS
    logger->log("Initializing buttons...");

    button1 = new ButtonHandler(PIN_BUTTON_1, true, true, LONG_PRESS_DURATION_MS, 250, BUTTON_DEBOUNCE_MS);
    button1->setOnClickCallback([this](int pinNumber, int clickCount) {
        logger->log("Button 1 clicked (count: %d)", clickCount);
        CounterApp::getInstance().increment();
    });
    button1->setOnLongPressStartCallback([this](int pinNumber) {
        logger->log("Button 1 long press - Enter pairing mode");
        if (!BLEManager::getInstance().isInPairingMode()) {
            BLEManager::getInstance().enterPairingMode();
        }
    });

    button2 = new ButtonHandler(PIN_BUTTON_2, true, true, LONG_PRESS_DURATION_MS, 250, BUTTON_DEBOUNCE_MS);
    button2->setOnClickCallback([this](int pinNumber, int clickCount) {
        logger->log("Button 2 clicked (count: %d)", clickCount);
        CounterApp::getInstance().decrement();
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
    if (!BLEManager::getInstance().begin(&CounterApp::getInstance())) {
        logger->log("FATAL: BLE initialization failed");
        currentState = SystemState::ERROR;
        return;
    }
}

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

            gfx.setTextSize(4);
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
