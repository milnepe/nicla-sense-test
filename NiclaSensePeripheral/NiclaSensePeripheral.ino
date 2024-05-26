/*
Arduino Nicla Sense ME peripheral test

Based on: Arduino Nicla Sense ME WEB BLE Sense dashboard demo
https://docs.arduino.cc/tutorials/nicla-sense-me/web-ble-dashboard/

Hardware required: https://store.arduino.cc/nicla-sense-me

Device sketch based on example by Sandeep Mistry and Massimo Banzi
Sketch and web dashboard copy-fixed to be used with the Nicla Sense ME by Pablo Marquínez

*/

#include "Nicla_System.h"
#include "Arduino_BHY2.h"
#include <ArduinoBLE.h>

// Standard IAQ levels
int GOOD = 50;
int MODERATE = 100;
int UNHEALTHY = 200;

// System state
enum state { NORMAL,
             MONITOR,
             REPLACE };

//----------------------------------------------------------------------------------------------------------------------
// BLE UUIDs
//----------------------------------------------------------------------------------------------------------------------

#define BLE_UUID_ENVIRONMENTAL_SENSING_SERVICE "181A"
#define BLE_UUID_DEVICE_ID_SERVICE "1800"
#define BLE_UUID_TEMPERATURE "2A6E"
#define BLE_UUID_HUMIDITY "2A6F"
#define BLE_UUID_PRESSURE "2A6D"
#define BLE_UUID_CARBON_DIOXIDE_SENSOR "054A"
#define BLE_UUID_AIR_QUALITY_SENSOR "0542"
#define BLE_UUID_GAS_SENSOR "107A"
#define BLE_UUID_STATTE "107D"
#define BLE_UUID_VERSION "2A28"


#define BLE_SENSE_UUID(val) ("19b10000-" val "-537e-4f6c-d104768a1214")

//----------------------------------------------------------------------------------------------------------------------
// BLE Data structures
//----------------------------------------------------------------------------------------------------------------------

typedef struct __attribute__((packed)) {
  int16_t temperature;
  uint16_t humidity;
  uint32_t pressure;
  uint16_t iaq;
  uint32_t co2;
  uint32_t gas;
  uint16_t state;
} nicla_env_data_t;

nicla_env_data_t niclaEnvData;

//----------------------------------------------------------------------------------------------------------------------
// BLE
//----------------------------------------------------------------------------------------------------------------------

const int VERSION = 0x00000001;
BLEService environmentalSensingService(BLE_UUID_ENVIRONMENTAL_SENSING_SERVICE);
BLEService service(BLE_SENSE_UUID("0000"));

// Must be a string - not working
BLEUnsignedIntCharacteristic versionCharacteristic(BLE_UUID_VERSION, BLERead);

BLEIntCharacteristic temperatureCharacteristic(BLE_UUID_TEMPERATURE, BLERead);
BLEUnsignedIntCharacteristic humidityCharacteristic(BLE_UUID_HUMIDITY, BLERead);
BLEUnsignedLongCharacteristic pressureCharacteristic(BLE_UUID_PRESSURE, BLERead);

BLEUnsignedIntCharacteristic iaqCharacteristic(BLE_UUID_AIR_QUALITY_SENSOR, BLERead);
BLEIntCharacteristic co2Characteristic(BLE_UUID_CARBON_DIOXIDE_SENSOR, BLERead);
BLEUnsignedIntCharacteristic gasCharacteristic(BLE_UUID_GAS_SENSOR, BLERead);

BLECharacteristic rgbLedCharacteristic(BLE_SENSE_UUID("8001"), BLERead | BLEWrite, 3 * sizeof(byte));  // Array of 3 bytes, RGB
BLEUnsignedIntCharacteristic stateCharacteristic(BLE_UUID_STATTE, BLERead);

// String to calculate the local and device name
String name;

Sensor temperature(SENSOR_ID_TEMP);
Sensor humidity(SENSOR_ID_HUM);
Sensor pressure(SENSOR_ID_BARO);
Sensor gas(SENSOR_ID_GAS);
SensorBSEC bsec(SENSOR_ID_BSEC);

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
  BLE.setAdvertisedService(environmentalSensingService);
  BLE.setAdvertisedService(service);

  // Add all the previously defined Characteristics
  environmentalSensingService.addCharacteristic(temperatureCharacteristic);
  environmentalSensingService.addCharacteristic(humidityCharacteristic);
  environmentalSensingService.addCharacteristic(pressureCharacteristic);
  service.addCharacteristic(versionCharacteristic);
  service.addCharacteristic(iaqCharacteristic);
  service.addCharacteristic(co2Characteristic);
  service.addCharacteristic(gasCharacteristic);
  service.addCharacteristic(rgbLedCharacteristic);
  service.addCharacteristic(stateCharacteristic);

  // Disconnect event handler
  BLE.setEventHandler(BLEDisconnected, blePeripheralDisconnectHandler);

  // Connect event handler
  BLE.setEventHandler(BLEConnected, blePeripheralConnectHandler);

  // Sensors event handlers
  temperatureCharacteristic.setEventHandler(BLERead, onTemperatureCharacteristicRead);
  humidityCharacteristic.setEventHandler(BLERead, onHumidityCharacteristicRead);
  pressureCharacteristic.setEventHandler(BLERead, onPressureCharacteristicRead);
  iaqCharacteristic.setEventHandler(BLERead, onIaqCharacteristic);
  co2Characteristic.setEventHandler(BLERead, onCo2CharacteristicRead);
  gasCharacteristic.setEventHandler(BLERead, onGasCharacteristicRead);
  stateCharacteristic.setEventHandler(BLERead, onStateCharacteristicRead);

  rgbLedCharacteristic.setEventHandler(BLEWritten, onRgbLedCharacteristicWrite);

  versionCharacteristic.setValue(VERSION);

  BLE.addService(environmentalSensingService);
  BLE.addService(service);
  BLE.advertise();
}

void loop() {
  static auto printTime = millis();

  // Standalone or BLE connected
  if (BLE.connected()) {
    BHY2.update(100);
  } else {
    BHY2.update();
  }

  if (millis() - printTime >= 1000) {
    printTime = millis();

    updateReadings();
    // Drive state using IAQ value
    updateState(niclaEnvData.iaq);
    plotReadings();
  }
}

void updateReadings() {
  // BLE defines Temperature UUID 2A6E Type sint16
  // Unit is in degrees Celsius with a resolution of 0.01 degrees Celsius
  niclaEnvData.temperature = round(temperature.value() * 100.0);

  // BLE defines Humidity UUID 2A6F Type uint16
  // Unit is in percent with a resolution of 0.01 percent
  niclaEnvData.humidity = round(humidity.value() * 100.0);

  // BLE defines Pressure UUID 2A6D Type uint32
  // Unit is in Pascal with a resolution of 0.1 Pa
  niclaEnvData.pressure = round(pressure.value() * 10.0);
  niclaEnvData.iaq = bsec.iaq();
  niclaEnvData.co2 = bsec.co2_eq();
  niclaEnvData.gas = gas.value();
}

// Compares value of reading agains defined levels
// to set the state and LED on Nicla
void updateState(uint16_t reading) {
  if (reading > MODERATE) {
    niclaEnvData.state = REPLACE;
    nicla::leds.setColor(red);
  } else if (reading > GOOD) {
    niclaEnvData.state = MONITOR;
    nicla::leds.setColor(blue);
  } else {  // NORMAL
    niclaEnvData.state = NORMAL;
    nicla::leds.setColor(green);
  }
}

void plotReadings() {
  Serial.print("IAQ:");
  Serial.print(niclaEnvData.iaq);
  Serial.print(",");
  Serial.print("CO2:");
  Serial.print(niclaEnvData.co2);
  Serial.print(",");
  Serial.print("Gas:");
  Serial.print(niclaEnvData.gas);
  Serial.print(",");
  Serial.print("Temperature:");
  Serial.print(niclaEnvData.temperature / 100.0);
  Serial.print(",");
  Serial.print("Humidity:");
  Serial.print(niclaEnvData.humidity / 100.0);
  Serial.print(",");
  Serial.print("Pressure:");
  Serial.print(niclaEnvData.pressure / 10.0);
  Serial.print(",");
  Serial.print("State:");
  Serial.println(niclaEnvData.state);
}

void blePeripheralDisconnectHandler(BLEDevice central) {
  nicla::leds.setColor(red);
}

void blePeripheralConnectHandler(BLEDevice central) {
  nicla::leds.setColor(green);
}

void onTemperatureCharacteristicRead(BLEDevice central, BLECharacteristic characteristic) {
  // Read temperature from buffer
  temperatureCharacteristic.writeValue(niclaEnvData.temperature);
}

void onHumidityCharacteristicRead(BLEDevice central, BLECharacteristic characteristic) {
  // Read humidity from buffer
  humidityCharacteristic.writeValue(niclaEnvData.humidity);
}

void onPressureCharacteristicRead(BLEDevice central, BLECharacteristic characteristic) {
  // Read pressure from buffer
  pressureCharacteristic.writeValue(niclaEnvData.pressure);
}

void onIaqCharacteristic(BLEDevice central, BLECharacteristic characteristic) {
  // Read air quality from buffer
  iaqCharacteristic.writeValue(niclaEnvData.iaq);
}

void onCo2CharacteristicRead(BLEDevice central, BLECharacteristic characteristic) {
  // Read CO2 from buffer
  co2Characteristic.writeValue(niclaEnvData.co2);
}

void onGasCharacteristicRead(BLEDevice central, BLECharacteristic characteristic) {
  // Read gas from buffer
  gasCharacteristic.writeValue(niclaEnvData.gas);
}

void onStateCharacteristicRead(BLEDevice central, BLECharacteristic characteristic) {
  // Read state from buffer
  stateCharacteristic.writeValue(niclaEnvData.state);
}

void onRgbLedCharacteristicWrite(BLEDevice central, BLECharacteristic characteristic) {
  byte r = rgbLedCharacteristic[0];
  byte g = rgbLedCharacteristic[1];
  byte b = rgbLedCharacteristic[2];

  nicla::leds.setColor(r, g, b);
}
