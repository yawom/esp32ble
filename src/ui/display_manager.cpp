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
    , lastRegisteredDeviceCount(0) {
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
                           const char* pairingPassword, size_t registeredDeviceCount) {
    if (!initialized) {
        return;
    }

    // Check if anything has changed
    bool stateChanged = (state != lastState);
    bool valuesChanged = (counterValue != lastCounterValue ||
                         deviceNearby != lastDeviceNearby ||
                         registeredDeviceCount != lastRegisteredDeviceCount);

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

    gfx.fillScreen(COLOR_BACKGROUND);
}

void DisplayManager::showNormalScreen(int32_t counterValue, bool deviceNearby, size_t registeredDeviceCount) {
    clear();

    // Header
    drawHeader("ESP32 Access Control", COLOR_ACCENT);

    // Status bar
    drawStatusBar(deviceNearby, registeredDeviceCount);

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
    gfx.setCursor(10, gfx.height() - 15);
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

void DisplayManager::drawStatusBar(bool deviceNearby, size_t registeredDeviceCount) {
    int y = 40;

    // Device nearby indicator
    gfx.setTextSize(FONT_SIZE_SMALL);
    if (deviceNearby) {
        gfx.setTextColor(COLOR_SUCCESS, COLOR_BACKGROUND);
        gfx.setCursor(10, y);
        gfx.print("Device: NEARBY");
    } else {
        gfx.setTextColor(COLOR_TEXT_SECONDARY, COLOR_BACKGROUND);
        gfx.setCursor(10, y);
        gfx.print("Device: ---");
    }

    // Registered devices count
    gfx.setTextColor(COLOR_TEXT_SECONDARY, COLOR_BACKGROUND);
    gfx.setCursor(gfx.width() - 80, y);
    gfx.print("Reg: ");
    gfx.print(registeredDeviceCount);
    gfx.print("/");
    gfx.print(MAX_REGISTERED_DEVICES);
}
