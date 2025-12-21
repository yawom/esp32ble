#include "display_manager.h"

DisplayManager& DisplayManager::getInstance() {
    static DisplayManager instance;
    return instance;
}

DisplayManager::DisplayManager()
    : tft(nullptr)
    , initialized(false)
    , lastUpdateTime(0) {
}

DisplayManager::~DisplayManager() {
    if (tft) {
        delete tft;
    }
}

bool DisplayManager::begin() {
    if (initialized) {
        return true;
    }

    DEBUG_PRINTLN("Initializing display...");

    tft = new TFT_eSPI();

    if (!tft) {
        DEBUG_PRINTLN("ERROR: Failed to create TFT instance");
        return false;
    }

    tft->init();
    tft->setRotation(TFT_ROTATION);
    tft->fillScreen(COLOR_BACKGROUND);
    tft->setTextColor(COLOR_TEXT_PRIMARY, COLOR_BACKGROUND);

    initialized = true;
    DEBUG_PRINTLN("Display initialized successfully");

    return true;
}

void DisplayManager::update(SystemState state, int32_t counterValue, bool deviceNearby,
                           const char* pairingPassword, size_t registeredDeviceCount) {
    if (!initialized) {
        return;
    }

    // Throttle updates
    unsigned long currentTime = millis();
    if (currentTime - lastUpdateTime < DISPLAY_UPDATE_INTERVAL_MS) {
        return;
    }
    lastUpdateTime = currentTime;

    // Render based on state
    switch (state) {
        case SystemState::INITIALIZING:
            showInitializingScreen();
            break;

        case SystemState::NORMAL:
            showNormalScreen(counterValue, deviceNearby, registeredDeviceCount);
            break;

        case SystemState::PAIRING_MODE:
            showPairingScreen(pairingPassword);
            break;

        case SystemState::ERROR:
            showErrorScreen();
            break;
    }
}

void DisplayManager::clear() {
    if (!initialized) {
        return;
    }

    tft->fillScreen(COLOR_BACKGROUND);
}

void DisplayManager::showNormalScreen(int32_t counterValue, bool deviceNearby, size_t registeredDeviceCount) {
    clear();

    // Header
    drawHeader("ESP32 Access Control", COLOR_ACCENT);

    // Status bar
    drawStatusBar(deviceNearby, registeredDeviceCount);

    // Counter value (large, centered)
    tft->setTextSize(FONT_SIZE_XLARGE);
    tft->setTextColor(COLOR_TEXT_PRIMARY, COLOR_BACKGROUND);

    String counterStr = String(counterValue);
    int textWidth = counterStr.length() * 24;  // Approximate width for size 4
    int x = (tft->width() - textWidth) / 2;
    int y = tft->height() / 2 - 20;

    tft->setCursor(x, y);
    tft->print(counterStr);

    // Label
    tft->setTextSize(FONT_SIZE_MEDIUM);
    tft->setTextColor(COLOR_TEXT_SECONDARY, COLOR_BACKGROUND);
    String label = "Counter Value";
    textWidth = label.length() * 12;  // Approximate width for size 2
    x = (tft->width() - textWidth) / 2;
    tft->setCursor(x, y + 50);
    tft->print(label);

    // Instructions
    tft->setTextSize(FONT_SIZE_SMALL);
    tft->setTextColor(COLOR_TEXT_SECONDARY, COLOR_BACKGROUND);
    tft->setCursor(10, tft->height() - 40);
    tft->print("BTN1: +/Pair(5s)");
    tft->setCursor(10, tft->height() - 25);
    tft->print("BTN2: -/Clear(5s)");
}

void DisplayManager::showPairingScreen(const char* password) {
    clear();

    // Header
    drawHeader("PAIRING MODE", COLOR_WARNING);

    // Pairing password (large, centered)
    tft->setTextSize(FONT_SIZE_XLARGE);
    tft->setTextColor(COLOR_WARNING, COLOR_BACKGROUND);

    String passwordStr = String(password ? password : "------");
    int textWidth = passwordStr.length() * 24;  // Approximate width for size 4
    int x = (tft->width() - textWidth) / 2;
    int y = tft->height() / 2 - 30;

    tft->setCursor(x, y);
    tft->print(passwordStr);

    // Instructions
    tft->setTextSize(FONT_SIZE_MEDIUM);
    tft->setTextColor(COLOR_TEXT_PRIMARY, COLOR_BACKGROUND);

    String inst1 = "Connect with";
    textWidth = inst1.length() * 12;
    x = (tft->width() - textWidth) / 2;
    tft->setCursor(x, y + 50);
    tft->print(inst1);

    String inst2 = "BLE Scanner app";
    textWidth = inst2.length() * 12;
    x = (tft->width() - textWidth) / 2;
    tft->setCursor(x, y + 70);
    tft->print(inst2);

    // Timeout info
    tft->setTextSize(FONT_SIZE_SMALL);
    tft->setTextColor(COLOR_TEXT_SECONDARY, COLOR_BACKGROUND);
    tft->setCursor(10, tft->height() - 25);
    tft->print("Timeout: 60 seconds");
}

void DisplayManager::showInitializingScreen() {
    clear();

    tft->setTextSize(FONT_SIZE_LARGE);
    tft->setTextColor(COLOR_ACCENT, COLOR_BACKGROUND);

    String text = "Initializing...";
    int textWidth = text.length() * 18;  // Approximate width for size 3
    int x = (tft->width() - textWidth) / 2;
    int y = tft->height() / 2 - 15;

    tft->setCursor(x, y);
    tft->print(text);

    // Show device name
    tft->setTextSize(FONT_SIZE_SMALL);
    tft->setTextColor(COLOR_TEXT_SECONDARY, COLOR_BACKGROUND);
    String deviceName = String(BLE_DEVICE_NAME);
    textWidth = deviceName.length() * 6;
    x = (tft->width() - textWidth) / 2;
    tft->setCursor(x, y + 40);
    tft->print(deviceName);
}

void DisplayManager::showErrorScreen() {
    clear();

    tft->setTextSize(FONT_SIZE_LARGE);
    tft->setTextColor(COLOR_ERROR, COLOR_BACKGROUND);

    String text = "ERROR";
    int textWidth = text.length() * 18;  // Approximate width for size 3
    int x = (tft->width() - textWidth) / 2;
    int y = tft->height() / 2 - 15;

    tft->setCursor(x, y);
    tft->print(text);

    // Error message
    tft->setTextSize(FONT_SIZE_SMALL);
    tft->setTextColor(COLOR_TEXT_SECONDARY, COLOR_BACKGROUND);
    String msg = "System Error";
    textWidth = msg.length() * 6;
    x = (tft->width() - textWidth) / 2;
    tft->setCursor(x, y + 40);
    tft->print(msg);
}

void DisplayManager::drawHeader(const char* title, uint16_t color) {
    // Draw header bar
    tft->fillRect(0, 0, tft->width(), 30, color);

    // Draw title
    tft->setTextSize(FONT_SIZE_MEDIUM);
    tft->setTextColor(COLOR_BACKGROUND, color);  // Inverted colors

    int textWidth = strlen(title) * 12;  // Approximate width for size 2
    int x = (tft->width() - textWidth) / 2;

    tft->setCursor(x, 8);
    tft->print(title);
}

void DisplayManager::drawStatusBar(bool deviceNearby, size_t registeredDeviceCount) {
    int y = 40;

    // Device nearby indicator
    tft->setTextSize(FONT_SIZE_SMALL);
    if (deviceNearby) {
        tft->setTextColor(COLOR_SUCCESS, COLOR_BACKGROUND);
        tft->setCursor(10, y);
        tft->print("Device: NEARBY");
    } else {
        tft->setTextColor(COLOR_TEXT_SECONDARY, COLOR_BACKGROUND);
        tft->setCursor(10, y);
        tft->print("Device: ---");
    }

    // Registered devices count
    tft->setTextColor(COLOR_TEXT_SECONDARY, COLOR_BACKGROUND);
    tft->setCursor(tft->width() - 80, y);
    tft->print("Reg: ");
    tft->print(registeredDeviceCount);
    tft->print("/");
    tft->print(MAX_REGISTERED_DEVICES);
}
