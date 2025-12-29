#include "counter_app.h"
#include "logger/Logger.h"

CounterApp& CounterApp::getInstance() {
    static CounterApp instance;
    return instance;
}

CounterApp::CounterApp()
    : counterValue(0)
    , deviceNearby(false)
    , registeredDeviceCount(0)
    , config(nullptr)
    , logger(nullptr) {
    memset(registeredDevices, 0, sizeof(registeredDevices));
}

CounterApp::~CounterApp() {
}

bool CounterApp::begin(IConfig* cfg, Logger* log) {
    if (!log) {
        return false;
    }

    logger = log;
    logger->log("Initializing Counter App...");

    if (!cfg) {
        logger->log("ERROR: Config is null");
        return false;
    }

    config = cfg;

    loadCounter();
    loadDevices();

    logger->log("Counter app initialized with value: %d", counterValue);
    logger->log("Registered devices: %zu", registeredDeviceCount);

    return true;
}

void CounterApp::increment() {
    counterValue++;
    BLEManager::getInstance().updateCounterValue(counterValue);
}

void CounterApp::decrement() {
    counterValue--;
    BLEManager::getInstance().updateCounterValue(counterValue);
}

void CounterApp::setValue(int32_t value) {
    counterValue = value;
    BLEManager::getInstance().updateCounterValue(counterValue);
}

void CounterApp::registerDevice(uint8_t* macAddress) {
    for (size_t i = 0; i < registeredDeviceCount; i++) {
        if (registeredDevices[i].isValid &&
            memcmp(registeredDevices[i].macAddress, macAddress, 6) == 0) {
            logger->log("Device already registered");
            return;
        }
    }

    if (registeredDeviceCount < MAX_REGISTERED_DEVICES) {
        registeredDevices[registeredDeviceCount].isValid = true;
        memcpy(registeredDevices[registeredDeviceCount].macAddress, macAddress, 6);
        registeredDevices[registeredDeviceCount].registeredTimestamp = millis();
        registeredDeviceCount++;

        logger->log("Device registered: %02X:%02X:%02X:%02X:%02X:%02X",
            macAddress[0], macAddress[1], macAddress[2],
            macAddress[3], macAddress[4], macAddress[5]);

    } else {
        logger->log("ERROR: Maximum number of registered devices reached");
    }
}

void CounterApp::clearAllDevices() {
    logger->log("Clearing all registered devices");

    BLEManager::getInstance().clearAllBonds();

    memset(registeredDevices, 0, sizeof(registeredDevices));
    registeredDeviceCount = 0;

    // Clear from config storage
    for (size_t i = 0; i < MAX_REGISTERED_DEVICES; i++) {
        char key[32];
        snprintf(key, sizeof(key), "%s.%zu.valid", CONFIG_DEVICES_PREFIX, i);
        config->setBool(key, false);
    }
    config->save();

    logger->log("All devices and BLE bonds cleared");
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
    logger->log("Device connected callback: %02X:%02X:%02X:%02X:%02X:%02X",
        macAddress[0], macAddress[1], macAddress[2],
        macAddress[3], macAddress[4], macAddress[5]);

    // If in pairing mode, register the device and allow connection
    if (BLEManager::getInstance().isInPairingMode()) {
        registerDevice(macAddress);
        deviceNearby = true;
        BLEManager::getInstance().updateProximityStatus(true);
        logger->log("Device registered and connected (pairing mode)");
        return;
    }

    // Check if device is authorized (registered)
    if (!isDeviceRegistered(macAddress)) {
        logger->log("UNAUTHORIZED: Device not registered");
        // Don't allow proximity status or operations
        // The device will be rejected at the characteristic level
        return;
    }

    // Device is authorized
    deviceNearby = true;
    BLEManager::getInstance().updateProximityStatus(true);
    logger->log("Authorized device connected");
}

void CounterApp::onDeviceDisconnected() {
    deviceNearby = false;
    logger->log("Device disconnected callback");

    // Update proximity status
    BLEManager::getInstance().updateProximityStatus(false);
}

void CounterApp::onCounterRead(int32_t& value) {
    // Verify authorization
    uint8_t macAddress[6];
    BLEManager::getInstance().getConnectedDeviceMAC(macAddress);

    if (!BLEManager::getInstance().isInPairingMode() && !isDeviceRegistered(macAddress)) {
        logger->log("UNAUTHORIZED: Read attempt from unregistered device");
        value = 0;
        return;
    }

    value = counterValue;
    logger->log("Counter read via BLE: %d", value);
}

void CounterApp::onCounterWrite(int32_t value) {
    // Verify authorization
    uint8_t macAddress[6];
    BLEManager::getInstance().getConnectedDeviceMAC(macAddress);

    if (!BLEManager::getInstance().isInPairingMode() && !isDeviceRegistered(macAddress)) {
        logger->log("UNAUTHORIZED: Write attempt from unregistered device");
        return;
    }

    logger->log("Counter write via BLE: %d", value);
    setValue(value);
}

void CounterApp::onPairingModeExit() {
    logger->log("Pairing mode exited - saving registered devices");
    saveDevices();
}

void CounterApp::saveCounter() {
    if (config) {
        config->setInt(CONFIG_COUNTER_VALUE, counterValue);
        config->save();
    }
}

void CounterApp::loadCounter() {
    if (config) {
        counterValue = config->getInt(CONFIG_COUNTER_VALUE, 0);
    }
}

void CounterApp::saveDevices() {
    if (!config) return;

    for (size_t i = 0; i < registeredDeviceCount; i++) {
        char key[32];

        // Save MAC address as string
        snprintf(key, sizeof(key), "%s.%zu.mac", CONFIG_DEVICES_PREFIX, i);
        char macStr[18];
        snprintf(macStr, sizeof(macStr), "%02X:%02X:%02X:%02X:%02X:%02X",
                 registeredDevices[i].macAddress[0], registeredDevices[i].macAddress[1],
                 registeredDevices[i].macAddress[2], registeredDevices[i].macAddress[3],
                 registeredDevices[i].macAddress[4], registeredDevices[i].macAddress[5]);
        config->setString(key, macStr);

        // Save timestamp
        snprintf(key, sizeof(key), "%s.%zu.timestamp", CONFIG_DEVICES_PREFIX, i);
        config->setInt(key, registeredDevices[i].registeredTimestamp);

        // Save valid flag
        snprintf(key, sizeof(key), "%s.%zu.valid", CONFIG_DEVICES_PREFIX, i);
        config->setBool(key, registeredDevices[i].isValid);
    }

    config->save();
}

void CounterApp::loadDevices() {
    if (!config) return;

    registeredDeviceCount = 0;

    for (size_t i = 0; i < MAX_REGISTERED_DEVICES; i++) {
        char key[32];

        // Check if device is valid
        snprintf(key, sizeof(key), "%s.%zu.valid", CONFIG_DEVICES_PREFIX, i);
        if (!config->getBool(key, false)) {
            continue;
        }

        // Load MAC address
        snprintf(key, sizeof(key), "%s.%zu.mac", CONFIG_DEVICES_PREFIX, i);
        std::string macStr = config->getString(key, "");
        if (macStr.empty()) {
            continue;
        }

        // Parse MAC address
        unsigned int mac[6];
        if (sscanf(macStr.c_str(), "%02X:%02X:%02X:%02X:%02X:%02X",
                   &mac[0], &mac[1], &mac[2], &mac[3], &mac[4], &mac[5]) == 6) {
            for (int j = 0; j < 6; j++) {
                registeredDevices[registeredDeviceCount].macAddress[j] = (uint8_t)mac[j];
            }

            // Load timestamp
            snprintf(key, sizeof(key), "%s.%zu.timestamp", CONFIG_DEVICES_PREFIX, i);
            registeredDevices[registeredDeviceCount].registeredTimestamp = config->getInt(key, 0);

            registeredDevices[registeredDeviceCount].isValid = true;
            registeredDeviceCount++;
        }
    }
}
