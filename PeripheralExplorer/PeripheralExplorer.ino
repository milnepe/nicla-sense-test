/*
  Arduino Nano 33 IoT Central test 
  
  Based on Peripheral Explorer

  This example scans for Bluetooth® Low Energy peripherals until one with a particular name ("LED")
  is found. Then connects, and discovers + prints all the peripheral's attributes.

  The circuit:
  - Arduino MKR WiFi 1010, Arduino Uno WiFi Rev2 board, Arduino Nano 33 IoT,
    Arduino Nano 33 BLE, or Arduino Nano 33 BLE Sense board.

  You can use it with another board that is compatible with this library and the
  Peripherals -> LED example.

  This example code is in the public domain.
*/

#include <ArduinoBLE.h>

void setup() {
  Serial.begin(9600);
  while (!Serial)
    ;

  // begin initialization
  if (!BLE.begin()) {
    Serial.println("starting Bluetooth® Low Energy module failed!");

    while (1)
      ;
  }

  Serial.println("Bluetooth® Low Energy Central - Peripheral Explorer");

  // start scanning for peripherals
  BLE.scan();
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
    if (peripheral.localName() == "NiclaSenseME-C059") {
      // stop scanning
      BLE.stopScan();

      explorerPeripheral(peripheral);

      // peripheral disconnected, we are done
      while (1) {
        // do nothing
      }
    }
  }
}

void explorerPeripheral(BLEDevice peripheral) {
  // connect to the peripheral
  Serial.println("Connecting ...");

  if (peripheral.connect()) {
    Serial.println("Connected");
  } else {
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

  // loop the services of the peripheral and explore each
  for (int i = 0; i < peripheral.serviceCount(); i++) {
    BLEService service = peripheral.service(i);

    exploreService(service);
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
      if (characteristic.uuid() == String("19b10000-2001-537e-4f6c-d104768a1214")) {
        float temperature = 0;
        BLECharateristic_to_value(characteristic, &temperature);
        Serial.print(" Temperature: ");
        Serial.print(temperature);
      }
      if (characteristic.uuid() == String("19b10000-3001-537e-4f6c-d104768a1214")) {
        uint8_t humidity = 0;
        BLECharateristic_to_value(characteristic, &humidity);
        Serial.print(" Humidity: ");
        Serial.print(humidity);
      }
      if (characteristic.uuid() == String("19b10000-4001-537e-4f6c-d104768a1214")) {
        float pressure = 0;
        BLECharateristic_to_value(characteristic, &pressure);
        Serial.print(" Pressure: ");
        Serial.print(pressure);
      }
      if (characteristic.uuid() == String("19b10000-9001-537e-4f6c-d104768a1214")) {
        float air_quality = 0;
        BLECharateristic_to_value(characteristic, &air_quality);
        Serial.print(" Air Quality: ");
        Serial.print(air_quality);
      }
      if (characteristic.uuid() == String("19b10000-9002-537e-4f6c-d104768a1214")) {
        uint32_t co2 = 0;
        BLECharateristic_to_value(characteristic, &co2);        
        Serial.print(" CO2: ");
        Serial.print(co2);
      }
      if (characteristic.uuid() == String("19b10000-9003-537e-4f6c-d104768a1214")) {
        uint32_t gas = 0;
        BLECharateristic_to_value(characteristic, &gas);  
        Serial.print(" Gas: ");
        Serial.print(gas);
      }
      Serial.print(", value 0x");
      printData(characteristic.value(), characteristic.valueLength());
    }
  }
  Serial.println();

  // loop the descriptors of the characteristic and explore each
  // for (int i = 0; i < characteristic.descriptorCount(); i++) {
  //   BLEDescriptor descriptor = characteristic.descriptor(i);

  //   exploreDescriptor(descriptor);
  // }
}

void exploreDescriptor(BLEDescriptor descriptor) {
  // print the UUID of the descriptor
  Serial.print("\t\tDescriptor ");
  Serial.print(descriptor.uuid());

  // read the descriptor value
  descriptor.read();

  // print out the value of the descriptor
  Serial.print(", value 0x");
  printData(descriptor.value(), descriptor.valueLength());

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