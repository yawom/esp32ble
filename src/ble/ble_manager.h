#ifndef BLE_MANAGER_H
#define BLE_MANAGER_H

#include "../config.h"
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>
#include <BLEClient.h>

// Forward declarations
class BLEManagerCallbacks {
public:
    virtual void onDeviceConnected(uint8_t* macAddress) = 0;
    virtual void onDeviceDisconnected() = 0;
    virtual void onCounterRead(int32_t& value) = 0;
    virtual void onCounterWrite(int32_t value) = 0;
};

class BLEManager {
public:
    static BLEManager& getInstance();

    // Initialize BLE stack
    bool begin(BLEManagerCallbacks* callbacks);

    // Start/stop advertising
    void startAdvertising();
    void stopAdvertising();

    // Pairing mode
    void enterPairingMode();
    void exitPairingMode();
    bool isInPairingMode() const { return pairingMode; }
    const char* getPairingPassword() const { return pairingPassword; }

    // Connection status
    bool isDeviceConnected() const { return deviceConnected; }
    void getConnectedDeviceMAC(uint8_t* macAddress);

    // Update characteristics
    void updateProximityStatus(bool isNearby);
    void updateCounterValue(int32_t value);

    // Check if device is authorized (registered)
    bool isDeviceAuthorized(uint8_t* macAddress, const RegisteredDevice devices[], size_t count);

    // Loop update (for pairing mode timeout)
    void update();

private:
    BLEManager();
    ~BLEManager();

    // Prevent copying
    BLEManager(const BLEManager&) = delete;
    BLEManager& operator=(const BLEManager&) = delete;

    // BLE objects
    BLEServer* server;
    BLEService* service;
    BLECharacteristic* counterCharacteristic;
    BLECharacteristic* proximityCharacteristic;
    BLECharacteristic* deviceNameCharacteristic;

    // State
    bool initialized;
    bool deviceConnected;
    bool pairingMode;
    char pairingPassword[7];  // 6 digits + null terminator
    unsigned long pairingModeStartTime;
    uint8_t connectedDeviceMAC[6];

    // Callbacks
    BLEManagerCallbacks* appCallbacks;

    // Helper functions
    void generatePairingPassword();
    void setupCharacteristics();

    // Internal callback classes
    friend class ServerCallbacks;
    friend class CounterCharacteristicCallbacks;
};

#endif // BLE_MANAGER_H
