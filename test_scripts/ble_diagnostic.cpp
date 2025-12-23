/**
 * BLE Diagnostic Test - Minimal BLE Advertisement
 *
 * This is a minimal test to verify BLE is working at the hardware level.
 * Upload this to your ESP32 to test if BLE radio is functioning.
 *
 * If this doesn't work, there may be a hardware or firmware issue.
 */

#include <Arduino.h>
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLEAdvertising.h>

void setup() {
    Serial.begin(115200);
    delay(2000);

    Serial.println("\n\n======================================");
    Serial.println("BLE DIAGNOSTIC TEST");
    Serial.println("======================================\n");

    // Initialize BLE with a simple name
    Serial.println("Initializing BLE...");
    BLEDevice::init("TEST-ESP32");

    // Print MAC address
    Serial.print("MAC Address: ");
    Serial.println(BLEDevice::getAddress().toString().c_str());

    // Create a simple server
    Serial.println("Creating BLE Server...");
    BLEServer* pServer = BLEDevice::createServer();

    // Start advertising immediately (no service needed for visibility test)
    Serial.println("Starting BLE Advertising...");
    BLEAdvertising* pAdvertising = BLEDevice::getAdvertising();
    pAdvertising->start();

    Serial.println("\n======================================");
    Serial.println("BLE ADVERTISING ACTIVE");
    Serial.println("======================================");
    Serial.println("Device Name: TEST-ESP32");
    Serial.println("Look for this device in your BLE scanner");
    Serial.println("======================================\n");
}

void loop() {
    // Print a heartbeat every 5 seconds to confirm it's running
    static unsigned long lastPrint = 0;
    if (millis() - lastPrint > 5000) {
        Serial.println("Still advertising... (Check your BLE scanner)");
        lastPrint = millis();
    }
    delay(1000);
}
