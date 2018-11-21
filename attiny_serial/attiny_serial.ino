//
//    FILE: dht_test.ino
//  AUTHOR: Rob Tillaart
// VERSION: 0.1.07
// PURPOSE: DHT Temperature & Humidity Sensor library for Arduino
//     URL: http://arduino.cc/playground/Main/DHTLib
//
// Released to the public domain
//

#include "dht.h"
#include <SoftwareSerial.h>
dht DHT;

#define DHT22_PIN 1
#define LED_PIN   4
#define rxPin 2
#define txPin 0
SoftwareSerial mySerial =  SoftwareSerial(rxPin, txPin);

double y = 0.0d;
void setup()
{

  pinMode(rxPin, INPUT);
  pinMode(txPin, OUTPUT);
  mySerial.begin(9600);

  mySerial.println("DHT TEST PROGRAM ");
  mySerial.print("LIBRARY VERSION: ");
  mySerial.println(DHT_LIB_VERSION);
  mySerial.println();
  mySerial.println("Type,\tstatus,\tHumidity (%),\tTemperature (C)");

  pinMode(LED_PIN, OUTPUT);
}

void loop()
{
  // READ DATA
  mySerial.print("DHT22, \t");
  int chk = DHT.read22(DHT22_PIN);
  switch (chk)
  {
    case DHTLIB_OK:
      mySerial.print("OK,\t");
      // DISPLAY DATA

      // offset equation from experiment
      y = DHT.humidity*0.839 - 6.378;
      mySerial.print(y, 1);
//      mySerial.print(DHT.humidity, 1);
      mySerial.print(",\t");
      mySerial.print(DHT.temperature, 1);
      //      if (DHT.humidity > 65.0) {
      //        digitalWrite(LED_PIN, HIGH);
      //      } else {
      //        digitalWrite(LED_PIN, LOW);
      //      }
      break;
    case DHTLIB_ERROR_CHECKSUM:
      mySerial.print("Checksum error,\t");
      //      for (int i = 0; i < 3; i++) {
      //        digitalWrite(LED_PIN, HIGH);
      //        delay(500);
      //        digitalWrite(LED_PIN, LOW);
      //        delay(500);
      //      }
      break;
    case DHTLIB_ERROR_TIMEOUT:
      mySerial.print("Time out error,\t");
      //      for (int i = 0; i < 10; i++) {
      //        digitalWrite(LED_PIN, HIGH);
      //        delay(100);
      //        digitalWrite(LED_PIN, LOW);
      //        delay(100);
      //      }
      break;
    default:
      mySerial.print("Unknown error,\t");
      break;
  }

  mySerial.println();

  delay(60000);

}
//
// END OF FILE
//
