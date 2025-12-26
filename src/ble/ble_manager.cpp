#include "ble_manager.h"
#include "logger/Logger.h"

// ============================================================================
// Server Callbacks
// ============================================================================

class ServerCallbacks : public BLEServerCallbacks {
private:
    BLEManager* manager;

public:
    ServerCallbacks(BLEManager* mgr) : manager(mgr) {}

    void onConnect(BLEServer* pServer) override {
        manager->deviceConnected = true;

        // Get connected device MAC address
        std::map<uint16_t, conn_status_t> connections = pServer->getPeerDevices(false);
        if (!connections.empty()) {
            auto it = connections.begin();
            BLEClient* pClient = static_cast<BLEClient*>(it->second.peer_device);
            if (pClient) {
                BLEAddress addr = pClient->getPeerAddress();
                memcpy(manager->connectedDeviceMAC, addr.getNative(), 6);

                logger->log("Device connected: %02X:%02X:%02X:%02X:%02X:%02X",
                    manager->connectedDeviceMAC[0], manager->connectedDeviceMAC[1],
                    manager->connectedDeviceMAC[2], manager->connectedDeviceMAC[3],
                    manager->connectedDeviceMAC[4], manager->connectedDeviceMAC[5]);

                if (manager->appCallbacks) {
                    manager->appCallbacks->onDeviceConnected(manager->connectedDeviceMAC);
                }
            }
        }
    }

    void onDisconnect(BLEServer* pServer) override {
        manager->deviceConnected = false;
        logger->log("Device disconnected");

        if (manager->appCallbacks) {
            manager->appCallbacks->onDeviceDisconnected();
        }

        // Restart advertising
        delay(500);
        manager->startAdvertising();
    }
};

// ============================================================================
// Counter Characteristic Callbacks
// ============================================================================

class CounterCharacteristicCallbacks : public BLECharacteristicCallbacks {
private:
    BLEManager* manager;

public:
    CounterCharacteristicCallbacks(BLEManager* mgr) : manager(mgr) {}

    void onRead(BLECharacteristic* pCharacteristic) override {
        if (manager->appCallbacks) {
            int32_t value = 0;
            manager->appCallbacks->onCounterRead(value);

            // Update the characteristic with current value
            pCharacteristic->setValue(value);
            logger->log("Counter read: %d", value);
        }
    }

    void onWrite(BLECharacteristic* pCharacteristic) override {
        std::string value = pCharacteristic->getValue();

        if (value.length() == sizeof(int32_t)) {
            int32_t counterValue;
            memcpy(&counterValue, value.data(), sizeof(int32_t));

            logger->log("Counter written: %d", counterValue);

            if (manager->appCallbacks) {
                manager->appCallbacks->onCounterWrite(counterValue);
            }
        } else {
            logger->log("WARNING: Invalid counter write size");
        }
    }
};

// ============================================================================
// BLEManager Implementation
// ============================================================================

BLEManager& BLEManager::getInstance() {
    static BLEManager instance;
    return instance;
}

BLEManager::BLEManager()
    : server(nullptr)
    , service(nullptr)
    , counterCharacteristic(nullptr)
    , proximityCharacteristic(nullptr)
    , deviceNameCharacteristic(nullptr)
    , initialized(false)
    , deviceConnected(false)
    , pairingMode(false)
    , pairingModeStartTime(0)
    , appCallbacks(nullptr) {
    memset(pairingPassword, 0, sizeof(pairingPassword));
    memset(connectedDeviceMAC, 0, sizeof(connectedDeviceMAC));
}

BLEManager::~BLEManager() {
    if (initialized) {
        stopAdvertising();
    }
}

bool BLEManager::begin(BLEManagerCallbacks* callbacks) {
    if (initialized) {
        logger->log("BLE already initialized");
        return true;
    }

    appCallbacks = callbacks;

    logger->log("\n====================================");
    logger->log("BLE INITIALIZATION STARTING");
    logger->log("====================================");

    // Initialize BLE
    logger->log("Initializing BLE device as: %s", BLE_DEVICE_NAME);
    BLEDevice::init(BLE_DEVICE_NAME);

    // Print Bluetooth MAC address
    std::string macAddr = BLEDevice::getAddress().toString();
    logger->log("Bluetooth MAC Address: %s", macAddr.c_str());

    // Create BLE Server
    logger->log("Creating BLE Server...");
    server = BLEDevice::createServer();
    server->setCallbacks(new ServerCallbacks(this));

    // Create BLE Service
    logger->log("Creating BLE Service with UUID: %s", SERVICE_UUID);
    service = server->createService(SERVICE_UUID);

    // Setup characteristics
    logger->log("Setting up BLE characteristics...");
    setupCharacteristics();

    // Start the service
    logger->log("Starting BLE service...");
    service->start();

    // Mark as initialized BEFORE starting advertising (advertising checks this flag)
    initialized = true;

    // Start advertising
    startAdvertising();

    logger->log("\n====================================");
    logger->log("BLE INITIALIZATION COMPLETE");
    logger->log("====================================");
    logger->log("Device visible as: %s", BLE_DEVICE_NAME);
    logger->log("====================================\n");

    return true;
}

void BLEManager::setupCharacteristics() {
    // Counter characteristic (Read/Write/Notify)
    logger->log("  - Counter Characteristic: %s", COUNTER_CHAR_UUID);
    counterCharacteristic = service->createCharacteristic(
        COUNTER_CHAR_UUID,
        BLECharacteristic::PROPERTY_READ |
        BLECharacteristic::PROPERTY_WRITE |
        BLECharacteristic::PROPERTY_NOTIFY
    );
    counterCharacteristic->addDescriptor(new BLE2902());
    counterCharacteristic->setCallbacks(new CounterCharacteristicCallbacks(this));

    // Proximity characteristic (Read/Notify)
    logger->log("  - Proximity Characteristic: %s", PROXIMITY_CHAR_UUID);
    proximityCharacteristic = service->createCharacteristic(
        PROXIMITY_CHAR_UUID,
        BLECharacteristic::PROPERTY_READ |
        BLECharacteristic::PROPERTY_NOTIFY
    );
    proximityCharacteristic->addDescriptor(new BLE2902());

    // Device name characteristic (Read)
    logger->log("  - Device Name Characteristic: %s", DEVICE_NAME_CHAR_UUID);
    deviceNameCharacteristic = service->createCharacteristic(
        DEVICE_NAME_CHAR_UUID,
        BLECharacteristic::PROPERTY_READ
    );
    deviceNameCharacteristic->setValue(BLE_DEVICE_NAME);

    logger->log("All characteristics configured successfully");
}

void BLEManager::startAdvertising() {
    if (!initialized) {
        logger->log("ERROR: Cannot start advertising - BLE not initialized");
        return;
    }

    logger->log("\n=== Starting BLE Advertising ===");

    BLEAdvertising* advertising = BLEDevice::getAdvertising();

    // Add service UUID to advertising data
    advertising->addServiceUUID(SERVICE_UUID);
    logger->log("Service UUID: %s", SERVICE_UUID);

    // Enable scan response for device name
    advertising->setScanResponse(true);

    // Set advertising parameters for better iOS compatibility
    advertising->setMinPreferred(0x06);  // Minimum connection interval
    advertising->setMaxPreferred(0x12);  // Maximum connection interval (was duplicate setMinPreferred)

    // Set advertising interval (in 0.625ms units, so 160 = 100ms)
    advertising->setMinInterval(160);
    advertising->setMaxInterval(160);

    // Explicitly set device as connectable and discoverable
    advertising->setAdvertisementType(ADV_TYPE_IND);

    BLEDevice::startAdvertising();

    logger->log("BLE Advertising Configuration:");
    logger->log("  Device Name: %s", BLE_DEVICE_NAME);
    logger->log("  Service UUID: %s", SERVICE_UUID);
    logger->log("  Scan Response: Enabled");
    logger->log("  Advertisement Type: Connectable & Discoverable (ADV_IND)");
    logger->log("  Status: ACTIVE");
    logger->log("================================\n");
}

void BLEManager::stopAdvertising() {
    if (!initialized) {
        return;
    }

    BLEDevice::stopAdvertising();
    logger->log("BLE advertising stopped");
}

void BLEManager::enterPairingMode() {
    pairingMode = true;
    pairingModeStartTime = millis();
    generatePairingPassword();

    logger->log("Entered pairing mode with password: %s", pairingPassword);
}

void BLEManager::exitPairingMode() {
    pairingMode = false;
    memset(pairingPassword, 0, sizeof(pairingPassword));

    logger->log("Exited pairing mode");
}

void BLEManager::generatePairingPassword() {
    // Generate a random 6-digit password
    for (int i = 0; i < 6; i++) {
        pairingPassword[i] = '0' + random(0, 10);
    }
    pairingPassword[6] = '\0';
}

void BLEManager::getConnectedDeviceMAC(uint8_t* macAddress) {
    memcpy(macAddress, connectedDeviceMAC, 6);
}

void BLEManager::updateProximityStatus(bool isNearby) {
    if (!initialized || !proximityCharacteristic) {
        return;
    }

    uint8_t value = isNearby ? 1 : 0;
    proximityCharacteristic->setValue(&value, 1);

    if (deviceConnected) {
        proximityCharacteristic->notify();
    }
}

void BLEManager::updateCounterValue(int32_t value) {
    if (!initialized || !counterCharacteristic) {
        return;
    }

    counterCharacteristic->setValue(value);

    if (deviceConnected) {
        counterCharacteristic->notify();
    }
}

bool BLEManager::isDeviceAuthorized(uint8_t* macAddress, const RegisteredDevice devices[], size_t count) {
    // In pairing mode, all devices are authorized
    if (pairingMode) {
        return true;
    }

    // Check if device is in the registered list
    for (size_t i = 0; i < count; i++) {
        if (devices[i].isValid && memcmp(devices[i].macAddress, macAddress, 6) == 0) {
            return true;
        }
    }

    return false;
}

void BLEManager::update() {
    // Check pairing mode timeout
    if (pairingMode) {
        if (millis() - pairingModeStartTime >= PAIRING_MODE_TIMEOUT_MS) {
            logger->log("Pairing mode timeout");
            exitPairingMode();
        }
    }
}
