# nicla-sense-test
Test code for Arduino Nicla Sense peripheral and Nano 33 IoT central

## NiclaSensePeripheral
A BLE peripheral that samples it's environmental sensors.

The on-board RGB indicates central connection status:

RED: Disconnected
GREEN: Connected

Open the Serial terminal to see output when sensors are queried:
```
Start
address = 9a:2b:c3:20:c0:59
name = NiclaSenseME-C059
Start
address = 9a:2b:c3:20:c0:59
name = NiclaSenseME-C059
Temperature: 30.66
Humidity: 38
Pressure: 1003.10
Air quality: 25.00
CO2: 500
Gas: 26522
```

## PeripheralExplorer
Central to query NiclaSensePeripheral.

Start the peripheral, open the serial monitor, then reset the Nano. It exits once the test completes.

Set peripheral name to match your Nicla id - this is how it makes the connection:
```
    if (peripheral.localName() == "NiclaSenseME-C059") {
      // stop scanning
      BLE.stopScan();
      ...
```
Example output:

```
Service 19b10000-0000-537e-4f6c-d104768a1214
	Characteristic 19b10000-2001-537e-4f6c-d104768a1214, properties 0x2 Temperature: 30.66, value 0xAE47F541
	Characteristic 19b10000-3001-537e-4f6c-d104768a1214, properties 0x2 Humidity: 38, value 0x26000000
	Characteristic 19b10000-4001-537e-4f6c-d104768a1214, properties 0x2 Pressure: 1003.10, value 0x1EC67A44
	Characteristic 19b10000-1001-537e-4f6c-d104768a1214, properties 0x2, value 0x00000000
	Characteristic 19b10000-9001-537e-4f6c-d104768a1214, properties 0x2 Air Quality: 25.00, value 0x0000C841
	Characteristic 19b10000-9002-537e-4f6c-d104768a1214, properties 0x2 CO2: 500, value 0xF4010000
	Characteristic 19b10000-9003-537e-4f6c-d104768a1214, properties 0x2 Gas: 26522, value 0x9A670000
	Characteristic 19b10000-8001-537e-4f6c-d104768a1214, properties 0xA
```
## ToDo
Modify the rgbLedCharacteristic to indicate the Red / Amber / Green status of the environment, based on some kind of logic.

