#ifndef STORAGE_MANAGER_H
#define STORAGE_MANAGER_H

#include "../config.h"
#include <LittleFS.h>

class StorageManager {
public:
    static StorageManager& getInstance();

    // Initialize the file system
    bool begin();

    // Counter persistence
    bool saveCounter(int32_t value);
    bool loadCounter(int32_t& value);

    // Registered devices management
    bool saveRegisteredDevices(const RegisteredDevice devices[], size_t count);
    bool loadRegisteredDevices(RegisteredDevice devices[], size_t& count, size_t maxDevices);
    bool clearRegisteredDevices();

    // Utility functions
    bool formatFileSystem();
    size_t getUsedSpace();
    size_t getTotalSpace();

private:
    StorageManager();
    ~StorageManager();

    // Prevent copying
    StorageManager(const StorageManager&) = delete;
    StorageManager& operator=(const StorageManager&) = delete;

    bool initialized;

    // Helper functions
    bool writeFile(const char* path, const uint8_t* data, size_t size);
    bool readFile(const char* path, uint8_t* data, size_t maxSize, size_t& bytesRead);
    bool deleteFile(const char* path);
};

#endif // STORAGE_MANAGER_H
