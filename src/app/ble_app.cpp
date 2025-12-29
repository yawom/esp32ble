#include "ble_app.h"

BLEApp::BLEApp()
    : ApplicationBase("ESP32-BLE-Demo", "1.0.0")
#if HAS_BUTTONS
    , button1(nullptr)
    , button2(nullptr)
#endif
#if HAS_DISPLAY
    , counterModule(nullptr)
#endif
    , currentState(SystemState::INITIALIZING) {
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

const char* BLEApp::getSystemTitle() {
    return "BLE Counter";
}

void BLEApp::onSetup() {
    setupButtons();

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

#if HAS_DISPLAY
    int systemModuleHeight = 30;
    int32_t startX = 0;
    int32_t startY = systemModuleHeight;
    int32_t width = display->getWidth();
    int32_t height = display->getHeight() - systemModuleHeight;

    counterModule = new CounterModule(logger);
    if (!moduleManager->registerModule(counterModule, startX, startY, width, height)) {
        logger->log("ERROR: Failed to register CounterModule");
        currentState = SystemState::ERROR;
        return;
    }

    if (!moduleManager->startAllModules()) {
        logger->log("ERROR: Failed to start all modules");
        currentState = SystemState::ERROR;
        return;
    }

    logger->log("CounterModule registered and started");
#endif

    currentState = SystemState::NORMAL;
    logger->log("BLE app initialized successfully!");
}

void BLEApp::onStateUpdate(AppState state) {
    if (state == AppState::MQTT_CONNECT) {
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
            BLEManager::getInstance().exitPairingMode();
            return;
        }

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
            BLEManager::getInstance().exitPairingMode();
            return;
        }

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
