/*
  Button

  Turns on and off a light emitting diode(LED) connected to digital pin 13,
  when pressing a pushbutton attached to pin 2.

  The circuit:
  - LED attached from pin 13 to ground
  - pushbutton attached to pin 2 from +5V
  - 10K resistor attached to pin 2 from ground

  - Note: on most Arduinos there is already an LED on the board
    attached to pin 13.

  created 2005
  by DojoDave <http://www.0j0.org>
  modified 30 Aug 2011
  by Tom Igoe

  This example code is in the public domain.

  http://www.arduino.cc/en/Tutorial/Button
*/

// constants won't change. They're used here to set pin numbers:
const int buttonPin = 3;     // the number of the pushbutton pin
const int ledPin =  13;      // the number of the LED pin

// variables will change:
int buttonState = 0;         // variable for reading the pushbutton status
uint8_t payloadByte = 2;

uint8_t counter = 0;
long myDelay = 50L;

bool volatile start = false;

void setup() {
  // initialize the LED pin as an output:
  pinMode(ledPin, OUTPUT);
  // initialize the pushbutton pin as an input:
  pinMode(buttonPin, INPUT);

  Serial.begin(19200);

  attachInterrupt(digitalPinToInterrupt(buttonPin), startSequence, RISING);
}

void loop() {

  if (start) {
    // turn LED on:
    digitalWrite(ledPin, HIGH);
    Serial.write(payloadByte + '0');
    //  Serial.print(payloadByte, DEC);
    delay(myDelay);
    digitalWrite(ledPin, LOW);
    delay(myDelay);
    if (counter++ >= 9) {
      counter = 0;
      if (payloadByte++ >= 9) {
        payloadByte = 0;
        start = false;
      }
    }
  }
}

void startSequence() {
  start = true;
}

