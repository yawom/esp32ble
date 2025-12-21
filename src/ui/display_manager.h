#ifndef DISPLAY_MANAGER_H
#define DISPLAY_MANAGER_H

#include "../config.h"
#include <TFT_eSPI.h>

class DisplayManager {
public:
    static DisplayManager& getInstance();

    // Initialize the display
    bool begin();

    // Update display based on system state
    void update(SystemState state, int32_t counterValue, bool deviceNearby,
                const char* pairingPassword = nullptr, size_t registeredDeviceCount = 0);

    // Clear the display
    void clear();

private:
    DisplayManager();
    ~DisplayManager();

    // Prevent copying
    DisplayManager(const DisplayManager&) = delete;
    DisplayManager& operator=(const DisplayManager&) = delete;

    TFT_eSPI* tft;
    bool initialized;
    unsigned long lastUpdateTime;

    // Screen rendering functions
    void showNormalScreen(int32_t counterValue, bool deviceNearby, size_t registeredDeviceCount);
    void showPairingScreen(const char* password);
    void showInitializingScreen();
    void showErrorScreen();

    // Helper functions
    void drawHeader(const char* title, uint16_t color);
    void drawStatusBar(bool deviceNearby, size_t registeredDeviceCount);
};

#endif // DISPLAY_MANAGER_H
