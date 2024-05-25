/*
Arduino Nicla Sense ME peripheral test

Based on: Arduino Nicla Sense ME WEB BLE Sense dashboard demo


Hardware required: https://store.arduino.cc/nicla-sense-me

1) Upload this sketch to the Arduino Nano BLE sense board

2) Open the following web page in the Chrome browser:
https://arduino.github.io/ArduinoAI/NiclaSenseME-dashboard/

3) Click on the green button in the web page to connect the browser to the board over BLE


Web dashboard by D. Pajak

Device sketch based on example by Sandeep Mistry and Massimo Banzi
Sketch and web dashboard copy-fixed to be used with the Nicla Sense ME by Pablo Marquínez

*/

#include "Nicla_System.h"
#include "Arduino_BHY2.h"
#include <ArduinoBLE.h>

#define BLE_SENSE_UUID(val) ("19b10000-" val "-537e-4f6c-d104768a1214")

const int VERSION = 0x00000000;

BLEService service(BLE_SENSE_UUID("0000"));

BLEUnsignedIntCharacteristic versionCharacteristic(BLE_SENSE_UUID("1001"), BLERead);
BLEFloatCharacteristic temperatureCharacteristic(BLE_SENSE_UUID("2001"), BLERead);
BLEUnsignedIntCharacteristic humidityCharacteristic(BLE_SENSE_UUID("3001"), BLERead);
BLEFloatCharacteristic pressureCharacteristic(BLE_SENSE_UUID("4001"), BLERead);

BLECharacteristic rgbLedCharacteristic(BLE_SENSE_UUID("8001"), BLERead | BLEWrite, 3 * sizeof(byte));  // Array of 3 bytes, RGB

BLEFloatCharacteristic bsecCharacteristic(BLE_SENSE_UUID("9001"), BLERead);
BLEIntCharacteristic co2Characteristic(BLE_SENSE_UUID("9002"), BLERead);
BLEUnsignedIntCharacteristic gasCharacteristic(BLE_SENSE_UUID("9003"), BLERead);

// String to calculate the local and device name
String name;

Sensor temperature(SENSOR_ID_TEMP);
Sensor humidity(SENSOR_ID_HUM);
Sensor pressure(SENSOR_ID_BARO);
Sensor gas(SENSOR_ID_GAS);
SensorBSEC bsec(SENSOR_ID_BSEC);

float temperatureValue = 0.0f;
uint8_t humidityValue = 0;
float pressureValue = 0.0f;
float airQuality = 0.0f;
uint32_t co2 = 0;
uint32_t g = 0;  // Gas sensor

void setup() {
  Serial.begin(115200);

  Serial.println("Start");

  nicla::begin();
  nicla::leds.begin();
  nicla::leds.setColor(red);

  //Sensors initialization
  BHY2.begin(NICLA_STANDALONE);
  temperature.begin();
  humidity.begin();
  pressure.begin();
  bsec.begin();
  gas.begin();
  // bsec.setTemperatureOffset(6.46);

  if (!BLE.begin()) {
    Serial.println("Failled to initialized BLE!");

    while (1)
      ;
  }

  String address = BLE.address();

  Serial.print("address = ");
  Serial.println(address);

  address.toUpperCase();

  name = "NiclaSenseME-";
  name += address[address.length() - 5];
  name += address[address.length() - 4];
  name += address[address.length() - 2];
  name += address[address.length() - 1];

  Serial.print("name = ");
  Serial.println(name);

  BLE.setLocalName(name.c_str());
  BLE.setDeviceName(name.c_str());
  BLE.setAdvertisedService(service);

  // Add all the previously defined Characteristics
  service.addCharacteristic(temperatureCharacteristic);
  service.addCharacteristic(humidityCharacteristic);
  service.addCharacteristic(pressureCharacteristic);
  service.addCharacteristic(versionCharacteristic);
  service.addCharacteristic(bsecCharacteristic);
  service.addCharacteristic(co2Characteristic);
  service.addCharacteristic(gasCharacteristic);
  service.addCharacteristic(rgbLedCharacteristic);

  // Disconnect event handler
  BLE.setEventHandler(BLEDisconnected, blePeripheralDisconnectHandler);

  // Connect event handler
  BLE.setEventHandler(BLEConnected, blePeripheralConnectHandler);

  // Sensors event handlers
  temperatureCharacteristic.setEventHandler(BLERead, onTemperatureCharacteristicRead);
  humidityCharacteristic.setEventHandler(BLERead, onHumidityCharacteristicRead);
  pressureCharacteristic.setEventHandler(BLERead, onPressureCharacteristicRead);
  bsecCharacteristic.setEventHandler(BLERead, onBsecCharacteristicRead);
  co2Characteristic.setEventHandler(BLERead, onCo2CharacteristicRead);
  gasCharacteristic.setEventHandler(BLERead, onGasCharacteristicRead);

  rgbLedCharacteristic.setEventHandler(BLEWritten, onRgbLedCharacteristicWrite);

  versionCharacteristic.setValue(VERSION);

  BLE.addService(service);
  BLE.advertise();
}

void loop() {
  static auto printTime = millis();

  if (BLE.connected()) {
    BHY2.update(100);
  }

  if (millis() - printTime >= 1000) {
    printTime = millis();

    updateReadings();
    plotReadings();
  }
}

void updateReadings() {
  temperatureValue = temperature.value();
  humidityValue = humidity.value() + 0.5f;  //since we are truncating the float type to a uint8_t, we want to round it
  pressureValue = pressure.value();
  airQuality = float(bsec.iaq());
  co2 = bsec.co2_eq();
  g = gas.value();
}

void plotReadings() {
  Serial.print("AQI:");
  Serial.print(airQuality);
  Serial.print(",");
  Serial.print("CO2:");
  Serial.print(co2);
  Serial.print(",");
  Serial.print("Gas:");
  Serial.println(g);
}

void blePeripheralDisconnectHandler(BLEDevice central) {
  nicla::leds.setColor(red);
}

void blePeripheralConnectHandler(BLEDevice central) {
  nicla::leds.setColor(green);
}

void onTemperatureCharacteristicRead(BLEDevice central, BLECharacteristic characteristic) {
  // Read temperature from buffer
  temperatureCharacteristic.writeValue(temperatureValue);
}

void onHumidityCharacteristicRead(BLEDevice central, BLECharacteristic characteristic) {
  // Read humidity from buffer
  humidityCharacteristic.writeValue(humidityValue);
}

void onPressureCharacteristicRead(BLEDevice central, BLECharacteristic characteristic) {
  // Read pressure from buffer
  pressureCharacteristic.writeValue(pressureValue);
}

void onBsecCharacteristicRead(BLEDevice central, BLECharacteristic characteristic) {
  // Read air quality from buffer
  bsecCharacteristic.writeValue(airQuality);
}

void onCo2CharacteristicRead(BLEDevice central, BLECharacteristic characteristic) {
  // Read CO2 from buffer
  co2Characteristic.writeValue(co2);
}

void onGasCharacteristicRead(BLEDevice central, BLECharacteristic characteristic) {
  // Read gas from buffer
  gasCharacteristic.writeValue(g);
}

void onRgbLedCharacteristicWrite(BLEDevice central, BLECharacteristic characteristic) {
  byte r = rgbLedCharacteristic[0];
  byte g = rgbLedCharacteristic[1];
  byte b = rgbLedCharacteristic[2];

  nicla::leds.setColor(r, g, b);
}
