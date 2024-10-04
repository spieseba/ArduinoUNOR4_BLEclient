
#include <Arduino.h>
#include <ArduinoBLE.h>
#include "WiFiS3.h"
#include "ble.h"

// Define the UUIDs for the service and the characteristic
constexpr char serviceUUID[] = "91bad492-b950-4226-aa2b-4ede9fa42f59";
constexpr char characteristicUUID[] = "cba1d466-344c-4be3-ab3f-189f80dd7518";

void startBLEStack()
{
  if (!BLE.begin())
  {
    Serial.println("Starting BLE failed!");
    while (1);
  }
  Serial.println("BLE started successfully on Arduino UNO R4 WiFi!");
  Serial.println("Scanning for devices ...");
  BLE.scanForUuid(serviceUUID);
}

void printBLEDeviceInfo(BLEDevice* device)
{
  Serial.print("Found device ");
  Serial.print(device->address());
  Serial.print(" '");
  Serial.print(device->localName());
  Serial.print("' ");
  Serial.print(device->advertisedServiceUuid());
  Serial.println();
}

void wrapperBLE(void (*wrappedFunc)(uint8_t))
{
  // Check if we have found a device
  BLEDevice device = BLE.available();  

  // Print info when device is found
  if (device)
    printBLEDeviceInfo(&device);
  else
    return;

  // Connect to the device
  Serial.print("Connecting ...");
  if (!device.connect()) 
  {
    Serial.println(" failed!");
    return;
  }
  Serial.println(" connected!");

  // Discover services on the connected device
  if (!device.discoverService(serviceUUID)) 
  {
    Serial.println("Failed to discover services.");
    device.disconnect();
    return;
  }
  // Try to access the service with the specified UUID
  BLEService discoveredService = device.service(serviceUUID);
  if (!discoveredService) 
  {
    Serial.println("Service not found!");
    device.disconnect();
    return;
  }

  // Try to access the characteristic with the specified UUID
  BLECharacteristic dataCharacteristic = device.characteristic(characteristicUUID);
  if (!dataCharacteristic) 
  {
    Serial.println("Characteristic not found!");
    device.disconnect();
    return;
  }
  // Subscribe to notifications from the characteristic
  if (!dataCharacteristic.canSubscribe()) 
  {
    Serial.println("Cannot subscribe to this characteristic");
    device.disconnect();
    return;
  }
  dataCharacteristic.subscribe();  // Subscribe to notifications
  Serial.println("Subscribed to characteristic");

  // While connected to device run wrappedFunc
  // separate BLE loop from call to wrappedFunc
  uint8_t dataBLE = 0;
  unsigned long updateTimeInterval = 2000; 
  unsigned long lastUpdateTime = millis();
  while (device.connected()) 
  {
    // BLE events are not processed automatically - need to call poll()
    BLE.poll();
    // Check if the characteristic has been updated
    if (dataCharacteristic.valueUpdated()) 
    {
      dataCharacteristic.readValue(&dataBLE, sizeof(dataBLE));
      Serial.print("Received value: ");
      Serial.println(dataBLE);
    }
    // Run game logic every updateTimeInterval
    if (millis() - lastUpdateTime > updateTimeInterval)
    {
      wrappedFunc(dataBLE);
      lastUpdateTime = millis();
    }
  }
  Serial.println("Disconnected!");
  return;
}
