/*

*/

#include <Wire.h>
#include "Adafruit_TCS34725.h"

#include <ArduinoBLE.h>

#define DATA_LENGTH 6
BLEService ledService("19B10000-E8F2-537E-4F6C-D104768A1218"); // create service

// create switch characteristic and allow remote device to read and write
BLEByteCharacteristic switchCharacteristic("19B10001-E8F2-537E-4F6C-D104768A1215", BLERead | BLEWrite);
BLECharacteristic hugeChar("19B10001-E8F2-537E-4F6C-D104768A1218", BLERead | BLEWrite , DATA_LENGTH, true);

const int ledPin = LED_BUILTIN; // pin to use for the LED
Adafruit_TCS34725 tcs = Adafruit_TCS34725(TCS34725_INTEGRATIONTIME_700MS, TCS34725_GAIN_1X);

void setup() {
  Serial.begin(115200);
  while (!Serial);

  if (tcs.begin()) {
    Serial.println("Found RGB sensor");
  } else {
    Serial.println("No TCS34725 found ... check your connections");
    while (1);
  }
  
  pinMode(ledPin, OUTPUT); // use the LED pin as an output

  // begin initialization
  if (!BLE.begin()) {
    Serial.println("starting BLE failed!");

    while (1);
  }

  // set the local name peripheral advertises
  BLE.setLocalName("LEDCallback");
  // set the UUID for the service this peripheral advertises
  BLE.setAdvertisedService(ledService);

  // add the characteristic to the service
  /*ledService.addCharacteristic(switchCharacteristic);*/
  ledService.addCharacteristic(hugeChar);

  // add service
  BLE.addService(ledService);

  // assign event handlers for connected, disconnected to peripheral
  BLE.setEventHandler(BLEConnected, blePeripheralConnectHandler);
  BLE.setEventHandler(BLEDisconnected, blePeripheralDisconnectHandler);

  // assign event handlers for characteristic
  hugeChar.setEventHandler(BLEWritten, hugeCharWritten);
  // set an initial value for the characteristic
  switchCharacteristic.setValue(0);

  // start advertising
  BLE.advertise();

  Serial.println(("Bluetooth device active, waiting for connections..."));
}

void loop() {
  uint16_t r, g, b, c, colorTemp, lux;

  tcs.getRawData(&r, &g, &b, &c);
  uint8_t data[DATA_LENGTH];

  data[0]=r;
  data[1]=g;
  data[2]=b;
  data[3]=0;
  data[4]=0;
  data[5]=0;
  hugeChar.writeValue(data, DATA_LENGTH);
  // poll for BLE events
  BLE.poll();
  
  Serial.printf("Raw R.%02d G.%02d B.%02d\n",r,g,b);
  
}

void blePeripheralConnectHandler(BLEDevice central) {
  // central connected event handler
  Serial.print("Connected event, central: ");
  Serial.println(central.address());
}

void blePeripheralDisconnectHandler(BLEDevice central) {
  // central disconnected event handler
  Serial.print("Disconnected event, central: ");
  Serial.println(central.address());
}

void hugeCharWritten(BLEDevice central, BLECharacteristic characteristic) {
  // central wrote new value to characteristic, update LED
  Serial.printf("Characteristic event, written: %s", characteristic.value());

  if (switchCharacteristic.value()) {
    Serial.println("LED on");
    digitalWrite(ledPin, HIGH);
  } else {
    Serial.println("LED off");
    digitalWrite(ledPin, LOW);
  }
}
