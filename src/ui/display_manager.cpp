#include "display_manager.h"
#include "esp32s3.h"

DisplayManager& DisplayManager::getInstance() {
    static DisplayManager instance;
    return instance;
}

DisplayManager::DisplayManager()
    : gfx()
    , initialized(false)
    , lastUpdateTime(0)
    , lastState(SystemState::INITIALIZING)
    , lastCounterValue(0)
    , lastDeviceNearby(false)
    , lastRegisteredDeviceCount(0)
    , lastBleConnected(false) {
}

DisplayManager::~DisplayManager() {
}

bool DisplayManager::begin() {
    if (initialized) {
        return true;
    }

    DEBUG_PRINTLN("Initializing display...");

#if defined(LILYGO_T_DISPLAY_S3)
    pinMode(PIN_POWER_ON, OUTPUT);
    digitalWrite(PIN_POWER_ON, HIGH);
#endif

    pinMode(PIN_LCD_BL, ANALOG);
    analogWrite(PIN_LCD_BL, 128);

    if (!gfx.init()) {
        DEBUG_PRINTLN("Display initialization failed");
        return false;
    }

    gfx.setRotation(TFT_ROTATION);
    gfx.fillScreen(COLOR_BACKGROUND);
    gfx.setTextColor(COLOR_TEXT_PRIMARY, COLOR_BACKGROUND);

    initialized = true;
    DEBUG_PRINTLN("Display initialized successfully");

    return true;
}

void DisplayManager::update(SystemState state, int32_t counterValue, bool deviceNearby,
                           const char* pairingPassword, size_t registeredDeviceCount, bool bleConnected) {
    if (!initialized) {
        return;
    }

    // Check if anything has changed
    bool stateChanged = (state != lastState);
    bool valuesChanged = (counterValue != lastCounterValue ||
                         deviceNearby != lastDeviceNearby ||
                         registeredDeviceCount != lastRegisteredDeviceCount ||
                         bleConnected != lastBleConnected);

    // Only update if something changed or enough time has passed
    unsigned long currentTime = millis();
    bool timeToUpdate = (currentTime - lastUpdateTime >= DISPLAY_UPDATE_INTERVAL_MS);

    if (!stateChanged && !valuesChanged && !timeToUpdate) {
        return;
    }

    // Update cache
    lastState = state;
    lastCounterValue = counterValue;
    lastDeviceNearby = deviceNearby;
    lastRegisteredDeviceCount = registeredDeviceCount;
    lastBleConnected = bleConnected;
    lastUpdateTime = currentTime;

    // If BLE is connected, show connected screen regardless of state (except pairing mode)
    if (bleConnected && state != SystemState::PAIRING_MODE) {
        showConnectedScreen(counterValue);
        return;
    }

    // Render based on state
    switch (state) {
        case SystemState::INITIALIZING:
            showInitializingScreen();
            break;

        case SystemState::NORMAL:
            showNormalScreen(counterValue, deviceNearby, registeredDeviceCount, bleConnected);
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

    gfx.fillScreen(COLOR_BACKGROUND);

    // Draw frame around screen to visualize boundaries
    gfx.drawRect(0, 0, gfx.width(), gfx.height(), COLOR_ACCENT);
    gfx.drawRect(1, 1, gfx.width() - 2, gfx.height() - 2, COLOR_ACCENT);
}

void DisplayManager::showNormalScreen(int32_t counterValue, bool deviceNearby, size_t registeredDeviceCount, bool bleConnected) {
    clear();

    // Header
    drawHeader("ESP32 Access Control", COLOR_ACCENT);

    // Status bar
    drawStatusBar(deviceNearby, registeredDeviceCount, bleConnected);

    // Counter value (large, centered)
    gfx.setTextSize(FONT_SIZE_XLARGE);
    gfx.setTextColor(COLOR_TEXT_PRIMARY, COLOR_BACKGROUND);

    String counterStr = String(counterValue);
    int textWidth = counterStr.length() * 24;  // Approximate width for size 4
    int x = (gfx.width() - textWidth) / 2;
    int y = 75;  // Fixed position from top

    gfx.setCursor(x, y);
    gfx.print(counterStr);

    // Label
    gfx.setTextSize(FONT_SIZE_MEDIUM);
    gfx.setTextColor(COLOR_TEXT_SECONDARY, COLOR_BACKGROUND);
    String label = "Counter Value";
    textWidth = label.length() * 12;  // Approximate width for size 2
    x = (gfx.width() - textWidth) / 2;
    gfx.setCursor(x, y + 50);
    gfx.print(label);

    // Instructions
}

void DisplayManager::showConnectedScreen(int32_t counterValue) {
    clear();

    // Header with success color
    drawHeader("BLE CONNECTED", COLOR_SUCCESS);

    // Large connected icon (using text)
    gfx.setTextSize(FONT_SIZE_XLARGE);
    gfx.setTextColor(COLOR_SUCCESS, COLOR_BACKGROUND);

    String icon = "OK";
    int textWidth = icon.length() * 24;
    int x = (gfx.width() - textWidth) / 2;
    int y = 50;

    gfx.setCursor(x, y);
    gfx.print(icon);

    // Counter value
    gfx.setTextSize(FONT_SIZE_LARGE);
    gfx.setTextColor(COLOR_TEXT_PRIMARY, COLOR_BACKGROUND);

    String counterStr = "Count: " + String(counterValue);
    textWidth = counterStr.length() * 18;
    x = (gfx.width() - textWidth) / 2;
    y = 100;

    gfx.setCursor(x, y);
    gfx.print(counterStr);

    // Instructions
    gfx.setTextSize(FONT_SIZE_SMALL);
    gfx.setTextColor(COLOR_TEXT_SECONDARY, COLOR_BACKGROUND);

    String inst1 = "Device is connected";
    textWidth = inst1.length() * 6;
    x = (gfx.width() - textWidth) / 2;
    gfx.setCursor(x, 140);
    gfx.print(inst1);

    String inst2 = "Use app to control";
    textWidth = inst2.length() * 6;
    x = (gfx.width() - textWidth) / 2;
    gfx.setCursor(x, 155);
    gfx.print(inst2);
}

void DisplayManager::showPairingScreen(const char* password) {
    clear();

    // Header
    drawHeader("PAIRING MODE", COLOR_WARNING);

    // Pairing password (large, centered)
    gfx.setTextSize(FONT_SIZE_XLARGE);
    gfx.setTextColor(COLOR_WARNING, COLOR_BACKGROUND);

    String passwordStr = String(password ? password : "------");
    int textWidth = passwordStr.length() * 24;  // Approximate width for size 4
    int x = (gfx.width() - textWidth) / 2;
    int y = 70;  // Fixed position from top

    gfx.setCursor(x, y);
    gfx.print(passwordStr);

    // Instructions
    gfx.setTextSize(FONT_SIZE_MEDIUM);
    gfx.setTextColor(COLOR_TEXT_PRIMARY, COLOR_BACKGROUND);

    String inst1 = "Connect with";
    textWidth = inst1.length() * 12;
    x = (gfx.width() - textWidth) / 2;
    gfx.setCursor(x, y + 50);
    gfx.print(inst1);

    String inst2 = "BLE Scanner app";
    textWidth = inst2.length() * 12;
    x = (gfx.width() - textWidth) / 2;
    gfx.setCursor(x, y + 70);
    gfx.print(inst2);

    // Timeout info
    gfx.setTextSize(FONT_SIZE_SMALL);
    gfx.setTextColor(COLOR_TEXT_SECONDARY, COLOR_BACKGROUND);
    gfx.setCursor(10, gfx.height() - 05);
    gfx.print("Timeout: 60 seconds");
}

void DisplayManager::showInitializingScreen() {
    clear();

    gfx.setTextSize(FONT_SIZE_LARGE);
    gfx.setTextColor(COLOR_ACCENT, COLOR_BACKGROUND);

    String text = "Initializing...";
    int textWidth = text.length() * 18;  // Approximate width for size 3
    int x = (gfx.width() - textWidth) / 2;
    int y = gfx.height() / 2 - 15;

    gfx.setCursor(x, y);
    gfx.print(text);

    // Show device name
    gfx.setTextSize(FONT_SIZE_SMALL);
    gfx.setTextColor(COLOR_TEXT_SECONDARY, COLOR_BACKGROUND);
    String deviceName = String(BLE_DEVICE_NAME);
    textWidth = deviceName.length() * 6;
    x = (gfx.width() - textWidth) / 2;
    gfx.setCursor(x, y + 40);
    gfx.print(deviceName);
}

void DisplayManager::showErrorScreen() {
    clear();

    gfx.setTextSize(FONT_SIZE_LARGE);
    gfx.setTextColor(COLOR_ERROR, COLOR_BACKGROUND);

    String text = "ERROR";
    int textWidth = text.length() * 18;  // Approximate width for size 3
    int x = (gfx.width() - textWidth) / 2;
    int y = gfx.height() / 2 - 15;

    gfx.setCursor(x, y);
    gfx.print(text);

    // Error message
    gfx.setTextSize(FONT_SIZE_SMALL);
    gfx.setTextColor(COLOR_TEXT_SECONDARY, COLOR_BACKGROUND);
    String msg = "System Error";
    textWidth = msg.length() * 6;
    x = (gfx.width() - textWidth) / 2;
    gfx.setCursor(x, y + 40);
    gfx.print(msg);
}

void DisplayManager::drawHeader(const char* title, uint16_t color) {
    // Draw header bar
    gfx.fillRect(0, 0, gfx.width(), 30, color);

    // Draw title
    gfx.setTextSize(FONT_SIZE_MEDIUM);
    gfx.setTextColor(COLOR_BACKGROUND, color);  // Inverted colors

    int textWidth = strlen(title) * 12;  // Approximate width for size 2
    int x = (gfx.width() - textWidth) / 2;

    gfx.setCursor(x, 8);
    gfx.print(title);
}

void DisplayManager::drawStatusBar(bool deviceNearby, size_t registeredDeviceCount, bool bleConnected) {
    int y = 40;

    // BLE connection status (left side)
    gfx.setTextSize(FONT_SIZE_SMALL);
    if (bleConnected) {
        gfx.setTextColor(COLOR_SUCCESS, COLOR_BACKGROUND);
        gfx.setCursor(10, y);
        gfx.print("BLE: CONNECTED");
    } else {
        gfx.setTextColor(COLOR_TEXT_SECONDARY, COLOR_BACKGROUND);
        gfx.setCursor(10, y);
        gfx.print("BLE: Waiting...");
    }

    // Registered devices count (right side)
    gfx.setTextColor(COLOR_TEXT_SECONDARY, COLOR_BACKGROUND);
    gfx.setCursor(gfx.width() - 80, y);
    gfx.print("Reg: ");
    gfx.print(registeredDeviceCount);
    gfx.print("/");
    gfx.print(MAX_REGISTERED_DEVICES);

    // Device nearby indicator (second line)
    if (bleConnected) {
        y += 15;
        if (deviceNearby) {
            gfx.setTextColor(COLOR_SUCCESS, COLOR_BACKGROUND);
            gfx.setCursor(10, y);
            gfx.print("Proximity: NEARBY");
        } else {
            gfx.setTextColor(COLOR_TEXT_SECONDARY, COLOR_BACKGROUND);
            gfx.setCursor(10, y);
            gfx.print("Proximity: Far");
        }
    }
}
