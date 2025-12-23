#ifndef DISPLAY_MANAGER_H
#define DISPLAY_MANAGER_H

#include "../config.h"
#include "display/board_display.h"

class DisplayManager {
public:
    static DisplayManager& getInstance();

    // Initialize the display
    bool begin();

    // Update display based on system state
    void update(SystemState state, int32_t counterValue, bool deviceNearby,
                const char* pairingPassword = nullptr, size_t registeredDeviceCount = 0,
                bool bleConnected = false);

    // Clear the display
    void clear();

private:
    DisplayManager();
    ~DisplayManager();

    // Prevent copying
    DisplayManager(const DisplayManager&) = delete;
    DisplayManager& operator=(const DisplayManager&) = delete;

    LGFX gfx;
    bool initialized;
    unsigned long lastUpdateTime;

    // Cache last displayed values to avoid unnecessary redraws
    SystemState lastState;
    int32_t lastCounterValue;
    bool lastDeviceNearby;
    size_t lastRegisteredDeviceCount;
    bool lastBleConnected;

    // Screen rendering functions
    void showNormalScreen(int32_t counterValue, bool deviceNearby, size_t registeredDeviceCount, bool bleConnected);
    void showPairingScreen(const char* password);
    void showInitializingScreen();
    void showErrorScreen();
    void showConnectedScreen(int32_t counterValue);

    // Helper functions
    void drawHeader(const char* title, uint16_t color);
    void drawStatusBar(bool deviceNearby, size_t registeredDeviceCount, bool bleConnected);
};

#endif // DISPLAY_MANAGER_H
