/*
  Arduino Nano 33 IoT BLE Central client for Arduino Nicla Sense ME

  Install the following libraries using the Arduino Libary Manager:
  ArduinoBLE https://github.com/arduino-libraries/ArduinoBLE
  Benoît Blanchon ArduinoJson https://arduinojson.org/
  Evert-arias EasyButton https://github.com/evert-arias/EasyButton

  Author: Peter Milne
  Date: 22 March 2023

  Copyright 2022 Peter Milne
  Released under GNU GENERAL PUBLIC LICENSE
  Version 3, 29 June 2007
*/

#include <EasyButton.h>
#include "NiclaAPI.h"
#include "NiclaMagnetDisplay.h"
#include "led.h"
#include "buzzer.h"
#include <ArduinoBLE.h>

// Button connections
#define B1_PIN 21
#define B2_PIN 20
#define B3_PIN 19
#define B4_PIN 18
#define B5_PIN 17
#define B6_PIN 16

modes mode = STD_MODE;

const char* soft_version = "0.1.0";

NiclaAPI myNiclaAPI = NiclaAPI();

NiclaMagnetDisplay epd = NiclaMagnetDisplay(&myNiclaAPI);

// int status = WL_IDLE_STATUS;

EasyButton button1(B1_PIN);
EasyButton button2(B2_PIN);
EasyButton button3(B3_PIN);
EasyButton button4(B4_PIN);
EasyButton button5(B5_PIN);
EasyButton button6(B6_PIN);

void setup() {
  led_init();
  buzzer_init();

  // Initialize Serial Port
  Serial.begin(115200);
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
  // delay(2000);

  Serial.print("Starting client version: ");
  Serial.println(soft_version);

  // Initialize buttons
  button1.begin();
  button1.onPressed(dry);
  button2.begin();
  button2.onPressed(rain);  // Place holder
  button3.begin();
  button3.onPressed(flood);  // Place holder
  button4.begin();
  button4.onPressed(replay);
  button4.onPressedFor(2000, buzzer_off);  // Cancel buzzer
  button5.begin();
  button6.begin();
  button6.onPressed(clock_sync_ap_mode);  // Place holder

  // Setup display and show greeting
  epd.initDisplay();
  epd.showGreeting();

  myNiclaAPI.init();

  // Hold down B5 while pressing reset to enter demo mode
  // Press reset to exit back to standard mode
  if (button5.isPressed()) {
    mode = DEMO_MODE;
    rgb_colour(RED);
    Serial.println("Starting demo mode...");
    doDemo();
  }
  delay(3000);
}

void loop() {
  // Continuously update the button states
  button1.read();
  button2.read();
  button3.read();
  button4.read();
  // button5.read(); Read only in setup
  button6.read();

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
      // stop scanning
      BLE.stopScan();

      rgb_colour(GREEN);
      epd.wifiOn = true;

      explorerPeripheral(peripheral);

      // peripheral disconnected, we are done
      while (1) {
        // do nothing
      }
    }
  } else {
    rgb_colour(RED);
    epd.wifiOn = false;
    // peripheral disconnected, we are done
    while (1) {
      // do nothing
    }
  }

  // static int status = WL_IDLE_STATUS;
  // if (WiFi.status() != WL_CONNECTED) {  // Connect wifi
  //   rgb_colour(RED);
  //   epd.wifiOn = false;
  //   reconnectWiFi();
  //   delay(2000);
  //   if (WiFi.status() == WL_CONNECTED) {
  //     rgb_colour(GREEN);
  //     epd.wifiOn = true;
  //     Serial.println("Wifi connected...");
  //     doUpdate();  // Initial update
  //   }
  // }
  // unsigned long now = millis();
  // static unsigned long lastApiAttemp = 0;
  // if ((now - lastApiAttemp > ALERT_INTERVAL) || (mode == REPLAY_MODE)) {
  //   mode = STD_MODE;  // Clear replay
  //   doUpdate();
  //   lastApiAttemp = now;
  // }
}

void doUpdate() {
  myNiclaAPI.getData();
  myNiclaAPI.updateState(myNiclaAPI.warning.severityLevel);
  epd.updateDisplay();
  printData();
}

void doDemo() {
  epd.demoOn = true;
  while (1) {
    button1.read();
    button2.read();
    button3.read();
    button4.read();
    // button5.read(); Read only in setup
    button6.read();
    unsigned long now = millis();
    static unsigned long lastUpdate = 0;
    if (now - lastUpdate > DEMO_INTERVAL) {
      buzzer_off();
      myNiclaAPI.demo(DEMO_MODE);
      epd.updateDisplay();
      lastUpdate = millis();
    }
  }
}

// int reconnectWiFi() {
//   // WL_IDLE_STATUS     = 0
//   // WL_NO_SSID_AVAIL   = 1
//   // WL_SCAN_COMPLETED  = 2
//   // WL_CONNECTED       = 3
//   // WL_CONNECT_FAILED  = 4
//   // WL_CONNECTION_LOST = 5
//   // WL_DISCONNECTED    = 6

//   WiFi.disconnect();  // Force a disconnect
//   delay(1000);
//   WiFi.begin(SECRET_SSID, SECRET_PASS);
//   return WiFi.status();
// }

// Button callbacks
void dry() {
  Serial.println("B1 button pressed...");
  bip();
}

void rain() {
  Serial.println("B2 button pressed...");
  bip();
}

void flood() {
  Serial.println("B3 button pressed...");
  bip();
}

void replay() {
  Serial.println("B4 button pressed...");
  mode = REPLAY_MODE;
  bip();
}

void buzzerOff() {
  Serial.println("B4 button held...");
  buzzer_off();
}

void clock_sync_ap_mode() {
  Serial.println("B6 button pressed...");
  bip();
}

// Debug output
void printData() {
  // Serial.print("Nicla Area: https://check-for-flooding.service.gov.uk/target-area/");
  // Serial.println(myNiclaAPI.warning.flood_area_id);

  Serial.print("Warning Level: ");
  Serial.println(myNiclaAPI.warning.severityLevel);

  Serial.print("Time Raised: ");
  Serial.println(myNiclaAPI.warning.time_raised);
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