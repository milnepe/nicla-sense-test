/*
  Arduino Nano 33 IoT BLE Central client for Arduino Nicla Sense ME

  Install the following libraries using the Arduino Libary Manager:
  ArduinoBLE https://github.com/arduino-libraries/ArduinoBLE

  Copyright 2024 Peter Milne
  Released under GNU GENERAL PUBLIC LICENSE
  Version 3, 29 June 2007
*/

#include "NiclaAPI.h"
#include "NiclaMagnetDisplay.h"
#include "led.h"
#include "buzzer.h"
#include <ArduinoBLE.h>

#define NICLA_NAME "NiclaSenseME-C059"

// Button connections - not used yet
#define B1_PIN 21
#define B2_PIN 20
#define B3_PIN 19
#define B4_PIN 18
#define B5_PIN 17
#define B6_PIN 16

const char *soft_version = "0.1.1";

NiclaAPI myNiclaAPI = NiclaAPI();

NiclaMagnetDisplay epd = NiclaMagnetDisplay(&myNiclaAPI);

void setup() {
  led_init();
  buzzer_init();

  Serial.begin(115200);
  while (!Serial)
    ;

  if (!BLE.begin()) {
    Serial.println("starting Bluetooth® Low Energy module failed!");

    while (1)
      ;
  }

  Serial.println("Bluetooth® Low Energy Central - Peripheral Explorer");

  BLE.scanForName(NICLA_NAME);

  Serial.print("Starting client version: ");
  Serial.println(soft_version);

  // Setup display and show greeting
  epd.initDisplay();
  epd.showGreeting();

  myNiclaAPI.init();

  delay(3000);
}

void loop() {
  // check if a peripheral has been discovered
  BLEDevice peripheral = BLE.available();

  if (peripheral) {
    // discovered a peripheral, print out address, local name, and advertised service
    Serial.print("Found ");
    Serial.print(peripheral.address());
    Serial.print(" '");
    Serial.print(peripheral.localName());
    Serial.print("' ");
    Serial.print(peripheral.advertisedServiceUuid());
    Serial.println();

    // see if peripheral matches
    if (peripheral.localName() == NICLA_NAME) {
      BLE.stopScan();

      explorerPeripheral(peripheral);  // Where the action happens
    }
  }
  // Try to reconnect
  rgb_colour(RED);
  epd.bleOn = false;
  delay(2000);
  BLE.scan();
}

void explorerPeripheral(BLEDevice peripheral) {
  // connect to the peripheral
  Serial.println("Connecting ...");

  if (peripheral.connect()) {
    rgb_colour(GREEN);
    epd.bleOn = true;
    epd.updateState();
    Serial.println("Connected");
  } else {
    rgb_colour(RED);
    epd.bleOn = false;
    Serial.println("Failed to connect!");
    return;
  }

  // discover peripheral attributes
  Serial.println("Discovering attributes ...");
  if (peripheral.discoverAttributes()) {
    Serial.println("Attributes discovered");
  } else {
    Serial.println("Attribute discovery failed!");
    peripheral.disconnect();
    return;
  }

  // read and print device name of peripheral
  Serial.println();
  Serial.print("Device name: ");
  Serial.println(peripheral.deviceName());
  Serial.print("Appearance: 0x");
  Serial.println(peripheral.appearance(), HEX);
  Serial.println();

  while (BLE.connected()) {
    // loop the services of the peripheral and explore each
    for (int i = 0; i < peripheral.serviceCount(); i++) {
      BLEService service = peripheral.service(i);

      exploreService(service);
    }
    myNiclaAPI.updateWarning(myNiclaAPI.data.severityLevel);
    epd.updateReadings();
    // while (1);
    delay(BLE_READ_INTERVAL);
  }

  Serial.println();

  // we are done exploring, disconnect
  Serial.println("Disconnecting ...");
  peripheral.disconnect();
  Serial.println("Disconnected");
}

void exploreService(BLEService service) {
  // print the UUID of the service
  Serial.print("Service ");
  Serial.println(service.uuid());

  // loop the characteristics of the service and explore each
  for (int i = 0; i < service.characteristicCount(); i++) {
    BLECharacteristic characteristic = service.characteristic(i);

    exploreCharacteristic(characteristic);
  }
}

void exploreCharacteristic(BLECharacteristic characteristic) {
  Serial.print("\tCharacteristic ");
  Serial.print(characteristic.uuid());
  Serial.print(", properties 0x");
  Serial.print(characteristic.properties(), HEX);

  // check if the characteristic is readable
  if (characteristic.canRead()) {
    // read the characteristic value
    characteristic.read();

    if (characteristic.valueLength() > 0) {
      if (characteristic.uuid() == String("2a6e")) {
        int16_t temperature = 0;
        BLECharateristic_to_value(characteristic, &temperature);
        myNiclaAPI.data.temperature = temperature;  // Value passed to display
        Serial.print(" Temperature: ");
        Serial.print(myNiclaAPI.data.temperature / 100.0);
      }
      if (characteristic.uuid() == String("2a6f")) {
        uint16_t humidity = 0;
        BLECharateristic_to_value(characteristic, &humidity);
        Serial.print(" Humidity: ");
        Serial.print(myNiclaAPI.data.humidity / 100.0);
      }
      if (characteristic.uuid() == String("2a6d")) {
        uint32_t pressure = 0;
        BLECharateristic_to_value(characteristic, &pressure);
        myNiclaAPI.data.pressure = pressure;
        Serial.print(" Pressure: ");
        Serial.print(myNiclaAPI.data.pressure / 10.0);
      }
      if (characteristic.uuid() == String("0542")) {
        uint16_t air_quality = 0;
        BLECharateristic_to_value(characteristic, &air_quality);
        myNiclaAPI.data.air_quality = air_quality;
        Serial.print(" Air Quality: ");
        Serial.print(myNiclaAPI.data.air_quality);
      }
      if (characteristic.uuid() == String("054a")) {
        uint32_t co2 = 0;
        BLECharateristic_to_value(characteristic, &co2);
        myNiclaAPI.data.co2 = co2;
        Serial.print(" CO2: ");
        Serial.print(myNiclaAPI.data.co2);
      }
      if (characteristic.uuid() == String("107a")) {
        uint32_t gas = 0;
        BLECharateristic_to_value(characteristic, &gas);
        Serial.print(" Gas: ");
        Serial.print(gas);
      }
      if (characteristic.uuid() == String("19b10000-9001-537e-4f6c-d104768a1214")) {
        uint16_t state = 0;
        BLECharateristic_to_value(characteristic, &state);
        myNiclaAPI.data.severityLevel = (warning_levels)state;
        Serial.print(" State: ");
        Serial.print(myNiclaAPI.data.severityLevel);
      }
      Serial.print(", value 0x");
      printData(characteristic.value(), characteristic.valueLength());
    }
  }

  Serial.println();
}

void printData(const unsigned char data[], int length) {
  for (int i = 0; i < length; i++) {
    unsigned char b = data[i];

    if (b < 16) {
      Serial.print("0");
    }

    Serial.print(b, HEX);
  }
}

void BLECharateristic_to_value(BLECharacteristic characteristic, void *value) {
  if (characteristic.valueLength() == 4) {
    char *buffer = (char *)value;
    buffer[0] = characteristic.value()[0];  //  MMSB
    buffer[1] = characteristic.value()[1];  //  MLSB
    buffer[2] = characteristic.value()[2];  //  LMSB
    buffer[3] = characteristic.value()[3];  //  LLSB
  }
}