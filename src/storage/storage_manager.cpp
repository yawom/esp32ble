#include "storage_manager.h"

StorageManager& StorageManager::getInstance() {
    static StorageManager instance;
    return instance;
}

StorageManager::StorageManager() : initialized(false) {
}

StorageManager::~StorageManager() {
    if (initialized) {
        LittleFS.end();
    }
}

bool StorageManager::begin() {
    if (initialized) {
        return true;
    }

    DEBUG_PRINTLN("Initializing LittleFS...");

    if (!LittleFS.begin(true)) {  // true = format on mount failure
        DEBUG_PRINTLN("ERROR: Failed to mount LittleFS");
        return false;
    }

    initialized = true;
    DEBUG_PRINTLN("LittleFS initialized successfully");
    DEBUG_PRINTF("Total space: %zu bytes\n", getTotalSpace());
    DEBUG_PRINTF("Used space: %zu bytes\n", getUsedSpace());

    return true;
}

bool StorageManager::saveCounter(int32_t value) {
    if (!initialized) {
        DEBUG_PRINTLN("ERROR: Storage not initialized");
        return false;
    }

    DEBUG_PRINTF("Saving counter value: %d\n", value);

    struct {
        uint8_t version;
        int32_t counterValue;
    } data;

    data.version = STORAGE_FORMAT_VERSION;
    data.counterValue = value;

    return writeFile(COUNTER_FILE_PATH, (uint8_t*)&data, sizeof(data));
}

bool StorageManager::loadCounter(int32_t& value) {
    if (!initialized) {
        DEBUG_PRINTLN("ERROR: Storage not initialized");
        return false;
    }

    struct {
        uint8_t version;
        int32_t counterValue;
    } data;

    size_t bytesRead = 0;
    if (!readFile(COUNTER_FILE_PATH, (uint8_t*)&data, sizeof(data), bytesRead)) {
        DEBUG_PRINTLN("Counter file not found, initializing to 0");
        value = 0;
        return saveCounter(0);  // Create the file with initial value
    }

    if (bytesRead != sizeof(data) || data.version != STORAGE_FORMAT_VERSION) {
        DEBUG_PRINTLN("WARNING: Invalid counter file format");
        value = 0;
        return false;
    }

    value = data.counterValue;
    DEBUG_PRINTF("Loaded counter value: %d\n", value);
    return true;
}

bool StorageManager::saveRegisteredDevices(const RegisteredDevice devices[], size_t count) {
    if (!initialized) {
        DEBUG_PRINTLN("ERROR: Storage not initialized");
        return false;
    }

    if (count > MAX_REGISTERED_DEVICES) {
        DEBUG_PRINTLN("ERROR: Too many devices to save");
        return false;
    }

    DEBUG_PRINTF("Saving %zu registered devices\n", count);

    // Calculate file size
    size_t fileSize = 1 + (count * sizeof(RegisteredDevice));  // version byte + devices
    uint8_t* buffer = new uint8_t[fileSize];

    if (!buffer) {
        DEBUG_PRINTLN("ERROR: Failed to allocate buffer");
        return false;
    }

    // Write version
    buffer[0] = STORAGE_FORMAT_VERSION;

    // Write devices
    memcpy(buffer + 1, devices, count * sizeof(RegisteredDevice));

    bool success = writeFile(DEVICES_FILE_PATH, buffer, fileSize);
    delete[] buffer;

    if (success) {
        DEBUG_PRINTLN("Registered devices saved successfully");
    }

    return success;
}

bool StorageManager::loadRegisteredDevices(RegisteredDevice devices[], size_t& count, size_t maxDevices) {
    if (!initialized) {
        DEBUG_PRINTLN("ERROR: Storage not initialized");
        return false;
    }

    size_t maxFileSize = 1 + (maxDevices * sizeof(RegisteredDevice));
    uint8_t* buffer = new uint8_t[maxFileSize];

    if (!buffer) {
        DEBUG_PRINTLN("ERROR: Failed to allocate buffer");
        return false;
    }

    size_t bytesRead = 0;
    if (!readFile(DEVICES_FILE_PATH, buffer, maxFileSize, bytesRead)) {
        DEBUG_PRINTLN("No registered devices file found");
        delete[] buffer;
        count = 0;
        return true;  // Not an error, just no devices registered yet
    }

    if (bytesRead < 1 || buffer[0] != STORAGE_FORMAT_VERSION) {
        DEBUG_PRINTLN("WARNING: Invalid devices file format");
        delete[] buffer;
        count = 0;
        return false;
    }

    // Calculate number of devices
    size_t deviceCount = (bytesRead - 1) / sizeof(RegisteredDevice);

    if (deviceCount > maxDevices) {
        DEBUG_PRINTLN("WARNING: More devices in file than buffer can hold");
        deviceCount = maxDevices;
    }

    // Copy devices
    memcpy(devices, buffer + 1, deviceCount * sizeof(RegisteredDevice));
    count = deviceCount;

    delete[] buffer;

    DEBUG_PRINTF("Loaded %zu registered devices\n", count);
    return true;
}

bool StorageManager::clearRegisteredDevices() {
    if (!initialized) {
        DEBUG_PRINTLN("ERROR: Storage not initialized");
        return false;
    }

    DEBUG_PRINTLN("Clearing all registered devices");
    return deleteFile(DEVICES_FILE_PATH);
}

bool StorageManager::formatFileSystem() {
    if (initialized) {
        LittleFS.end();
        initialized = false;
    }

    DEBUG_PRINTLN("Formatting file system...");

    if (!LittleFS.begin(true)) {
        DEBUG_PRINTLN("ERROR: Failed to format and mount LittleFS");
        return false;
    }

    initialized = true;
    DEBUG_PRINTLN("File system formatted successfully");
    return true;
}

size_t StorageManager::getUsedSpace() {
    if (!initialized) {
        return 0;
    }
    return LittleFS.usedBytes();
}

size_t StorageManager::getTotalSpace() {
    if (!initialized) {
        return 0;
    }
    return LittleFS.totalBytes();
}

bool StorageManager::writeFile(const char* path, const uint8_t* data, size_t size) {
    File file = LittleFS.open(path, FILE_WRITE);

    if (!file) {
        DEBUG_PRINTF("ERROR: Failed to open file for writing: %s\n", path);
        return false;
    }

    size_t bytesWritten = file.write(data, size);
    file.close();

    if (bytesWritten != size) {
        DEBUG_PRINTF("ERROR: Write size mismatch. Expected: %zu, Written: %zu\n", size, bytesWritten);
        return false;
    }

    return true;
}

bool StorageManager::readFile(const char* path, uint8_t* data, size_t maxSize, size_t& bytesRead) {
    if (!LittleFS.exists(path)) {
        return false;
    }

    File file = LittleFS.open(path, FILE_READ);

    if (!file) {
        DEBUG_PRINTF("ERROR: Failed to open file for reading: %s\n", path);
        return false;
    }

    size_t fileSize = file.size();

    if (fileSize > maxSize) {
        DEBUG_PRINTF("ERROR: File too large. File size: %zu, Buffer size: %zu\n", fileSize, maxSize);
        file.close();
        return false;
    }

    bytesRead = file.read(data, fileSize);
    file.close();

    return (bytesRead == fileSize);
}

bool StorageManager::deleteFile(const char* path) {
    if (!LittleFS.exists(path)) {
        return true;  // File doesn't exist, consider it deleted
    }

    return LittleFS.remove(path);
}
