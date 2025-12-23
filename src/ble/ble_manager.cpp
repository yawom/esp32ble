#include "ble_manager.h"

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

                DEBUG_PRINTF("Device connected: %02X:%02X:%02X:%02X:%02X:%02X\n",
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
        DEBUG_PRINTLN("Device disconnected");

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
            DEBUG_PRINTF("Counter read: %d\n", value);
        }
    }

    void onWrite(BLECharacteristic* pCharacteristic) override {
        std::string value = pCharacteristic->getValue();

        if (value.length() == sizeof(int32_t)) {
            int32_t counterValue;
            memcpy(&counterValue, value.data(), sizeof(int32_t));

            DEBUG_PRINTF("Counter written: %d\n", counterValue);

            if (manager->appCallbacks) {
                manager->appCallbacks->onCounterWrite(counterValue);
            }
        } else {
            DEBUG_PRINTLN("WARNING: Invalid counter write size");
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
        return true;
    }

    appCallbacks = callbacks;

    DEBUG_PRINTLN("Initializing BLE...");

    // Initialize BLE
    BLEDevice::init(BLE_DEVICE_NAME);

    // Create BLE Server
    server = BLEDevice::createServer();
    server->setCallbacks(new ServerCallbacks(this));

    // Create BLE Service
    service = server->createService(SERVICE_UUID);

    // Setup characteristics
    setupCharacteristics();

    // Start the service
    service->start();

    // Start advertising
    startAdvertising();

    initialized = true;
    DEBUG_PRINTLN("BLE initialized successfully");

    return true;
}

void BLEManager::setupCharacteristics() {
    // Counter characteristic (Read/Write)
    counterCharacteristic = service->createCharacteristic(
        COUNTER_CHAR_UUID,
        BLECharacteristic::PROPERTY_READ |
        BLECharacteristic::PROPERTY_WRITE |
        BLECharacteristic::PROPERTY_NOTIFY
    );
    counterCharacteristic->addDescriptor(new BLE2902());
    counterCharacteristic->setCallbacks(new CounterCharacteristicCallbacks(this));

    // Proximity characteristic (Read/Notify)
    proximityCharacteristic = service->createCharacteristic(
        PROXIMITY_CHAR_UUID,
        BLECharacteristic::PROPERTY_READ |
        BLECharacteristic::PROPERTY_NOTIFY
    );
    proximityCharacteristic->addDescriptor(new BLE2902());

    // Device name characteristic (Read)
    deviceNameCharacteristic = service->createCharacteristic(
        DEVICE_NAME_CHAR_UUID,
        BLECharacteristic::PROPERTY_READ
    );
    deviceNameCharacteristic->setValue(BLE_DEVICE_NAME);
}

void BLEManager::startAdvertising() {
    if (!initialized) {
        return;
    }

    BLEAdvertising* advertising = BLEDevice::getAdvertising();
    advertising->addServiceUUID(SERVICE_UUID);
    advertising->setScanResponse(true);
    advertising->setMinPreferred(0x06);  // Functions that help with iPhone connections
    advertising->setMinPreferred(0x12);

    BLEDevice::startAdvertising();
    DEBUG_PRINTLN("BLE advertising started");
}

void BLEManager::stopAdvertising() {
    if (!initialized) {
        return;
    }

    BLEDevice::stopAdvertising();
    DEBUG_PRINTLN("BLE advertising stopped");
}

void BLEManager::enterPairingMode() {
    pairingMode = true;
    pairingModeStartTime = millis();
    generatePairingPassword();

    DEBUG_PRINTF("Entered pairing mode with password: %s\n", pairingPassword);
}

void BLEManager::exitPairingMode() {
    pairingMode = false;
    memset(pairingPassword, 0, sizeof(pairingPassword));

    DEBUG_PRINTLN("Exited pairing mode");
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
            DEBUG_PRINTLN("Pairing mode timeout");
            exitPairingMode();
        }
    }
}
