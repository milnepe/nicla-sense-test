/* 
 * This sketch shows how an arduino board can act as a host for nicla. 
 * An host board can configure the sensors of nicla and then read their values.
 * The host board should be connected to nicla through the eslov connector.
 * 
 * In this example, the temperature sensor is enabled and its
 * values are periodically read and then printed to the serial channel
 * 
 * NOTE: if Nicla is used as a Shield on top of a MKR board,
 * please use BHY2Host.begin(false, NICLA_AS_SHIELD)
*/

// Update Arduino_BHY2Host from http://github.com/arduino-libraries/Arduino_BHY2Host
// Fixes issue with sensors/DataParser.h
#include "Arduino_BHY2Host.h"

SensorBSEC bsec(SENSOR_ID_BSEC);

void setup() {
  Serial.begin(115200);
  while (!Serial)
    ;

  BHY2Host.begin(false, NICLA_VIA_BLE);

  bsec.begin();
  delay(2000);
  Serial.println("Starting...");
}

void loop() {
  static auto printTime = millis();
  BHY2Host.update();

  if (millis() - printTime >= 1000) {
    printTime = millis();
    // Serial.println(String("IAQ: ") + bsec.toString());
    Serial.println(String("IAQ: ") + bsec.iaq());
  }
}
