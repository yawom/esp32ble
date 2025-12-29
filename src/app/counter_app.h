#ifndef COUNTER_APP_H
#define COUNTER_APP_H

#include "../config.h"
#include "../ble/ble_manager.h"
#include "config/IConfig.h"
#include "logger/Logger.h"

class CounterApp : public BLEManagerCallbacks {
public:
    static CounterApp& getInstance();

    bool begin(IConfig* config, Logger* log);

    // Counter operations
    void increment();
    void decrement();
    void setValue(int32_t value);
    int32_t getValue() const { return counterValue; }

    // Registered devices management
    void registerDevice(uint8_t* macAddress);
    void clearAllDevices();
    size_t getRegisteredDeviceCount() const { return registeredDeviceCount; }
    const RegisteredDevice* getRegisteredDevices() const { return registeredDevices; }

    // Check if a device is registered
    bool isDeviceRegistered(uint8_t* macAddress);

    // BLE callback implementations
    void onDeviceConnected(uint8_t* macAddress) override;
    void onDeviceDisconnected() override;
    void onCounterRead(int32_t& value) override;
    void onCounterWrite(int32_t value) override;
    void onPairingModeExit() override;

    // Proximity detection
    bool isConnectedDeviceNearby() const { return deviceNearby; }

private:
    CounterApp();
    ~CounterApp();

    // Prevent copying
    CounterApp(const CounterApp&) = delete;
    CounterApp& operator=(const CounterApp&) = delete;

    int32_t counterValue;
    bool deviceNearby;
    RegisteredDevice registeredDevices[MAX_REGISTERED_DEVICES];
    size_t registeredDeviceCount;
    IConfig* config;
    Logger* logger;

    // Helper functions
    void saveCounter();
    void loadCounter();
    void saveDevices();
    void loadDevices();
};

#endif // COUNTER_APP_H
