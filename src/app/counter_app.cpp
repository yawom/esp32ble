#include "counter_app.h"

CounterApp& CounterApp::getInstance() {
    static CounterApp instance;
    return instance;
}

CounterApp::CounterApp()
    : counterValue(0)
    , deviceNearby(false)
    , registeredDeviceCount(0) {
    memset(registeredDevices, 0, sizeof(registeredDevices));
}

CounterApp::~CounterApp() {
}

bool CounterApp::begin() {
    DEBUG_PRINTLN("Initializing Counter App...");

    // Load persisted data
    loadCounter();
    loadDevices();

    DEBUG_PRINTF("Counter app initialized with value: %d\n", counterValue);
    DEBUG_PRINTF("Registered devices: %zu\n", registeredDeviceCount);

    return true;
}

void CounterApp::increment() {
    counterValue++;
    DEBUG_PRINTF("Counter incremented to: %d\n", counterValue);

    saveCounter();

    // Notify BLE clients
    BLEManager::getInstance().updateCounterValue(counterValue);
}

void CounterApp::decrement() {
    counterValue--;
    DEBUG_PRINTF("Counter decremented to: %d\n", counterValue);

    saveCounter();

    // Notify BLE clients
    BLEManager::getInstance().updateCounterValue(counterValue);
}

void CounterApp::setValue(int32_t value) {
    counterValue = value;
    DEBUG_PRINTF("Counter set to: %d\n", counterValue);

    saveCounter();

    // Notify BLE clients
    BLEManager::getInstance().updateCounterValue(counterValue);
}

void CounterApp::registerDevice(uint8_t* macAddress) {
    // Check if device is already registered
    for (size_t i = 0; i < registeredDeviceCount; i++) {
        if (registeredDevices[i].isValid &&
            memcmp(registeredDevices[i].macAddress, macAddress, 6) == 0) {
            DEBUG_PRINTLN("Device already registered");
            return;
        }
    }

    // Find a free slot
    if (registeredDeviceCount < MAX_REGISTERED_DEVICES) {
        registeredDevices[registeredDeviceCount].isValid = true;
        memcpy(registeredDevices[registeredDeviceCount].macAddress, macAddress, 6);
        registeredDevices[registeredDeviceCount].registeredTimestamp = millis();
        registeredDeviceCount++;

        DEBUG_PRINTF("Device registered: %02X:%02X:%02X:%02X:%02X:%02X\n",
            macAddress[0], macAddress[1], macAddress[2],
            macAddress[3], macAddress[4], macAddress[5]);

        saveDevices();
    } else {
        DEBUG_PRINTLN("ERROR: Maximum number of registered devices reached");
    }
}

void CounterApp::clearAllDevices() {
    DEBUG_PRINTLN("Clearing all registered devices");

    memset(registeredDevices, 0, sizeof(registeredDevices));
    registeredDeviceCount = 0;

    StorageManager::getInstance().clearRegisteredDevices();
}

bool CounterApp::isDeviceRegistered(uint8_t* macAddress) {
    for (size_t i = 0; i < registeredDeviceCount; i++) {
        if (registeredDevices[i].isValid &&
            memcmp(registeredDevices[i].macAddress, macAddress, 6) == 0) {
            return true;
        }
    }
    return false;
}

void CounterApp::onDeviceConnected(uint8_t* macAddress) {
    deviceNearby = true;

    DEBUG_PRINTF("Device connected callback: %02X:%02X:%02X:%02X:%02X:%02X\n",
        macAddress[0], macAddress[1], macAddress[2],
        macAddress[3], macAddress[4], macAddress[5]);

    // Update proximity status
    BLEManager::getInstance().updateProximityStatus(true);

    // If in pairing mode, register the device
    if (BLEManager::getInstance().isInPairingMode()) {
        registerDevice(macAddress);
    }
}

void CounterApp::onDeviceDisconnected() {
    deviceNearby = false;
    DEBUG_PRINTLN("Device disconnected callback");

    // Update proximity status
    BLEManager::getInstance().updateProximityStatus(false);
}

void CounterApp::onCounterRead(int32_t& value) {
    value = counterValue;
    DEBUG_PRINTF("Counter read via BLE: %d\n", value);
}

void CounterApp::onCounterWrite(int32_t value) {
    DEBUG_PRINTF("Counter write via BLE: %d\n", value);
    setValue(value);
}

void CounterApp::saveCounter() {
    StorageManager::getInstance().saveCounter(counterValue);
}

void CounterApp::loadCounter() {
    StorageManager::getInstance().loadCounter(counterValue);
}

void CounterApp::saveDevices() {
    StorageManager::getInstance().saveRegisteredDevices(registeredDevices, registeredDeviceCount);
}

void CounterApp::loadDevices() {
    StorageManager::getInstance().loadRegisteredDevices(
        registeredDevices,
        registeredDeviceCount,
        MAX_REGISTERED_DEVICES
    );
}
