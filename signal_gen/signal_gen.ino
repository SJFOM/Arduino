/*
SIGNAL_GENERATOR
 - A very simple program to generate a 0->5V square wave signal

See the following link on port manipulation: https://www.arduino.cc/en/Reference/PortManipulation

Date: 21 Nov 2018
*/

int sigPin = 3;
float frequency = 1000.0f;  // in Hz
float dutyCycle = 50.0f;    // in %

float timePeriod = 1000000.0f / frequency; // in us

int onTime = timePeriod * dutyCycle/100.0f;
int offTime = timePeriod - onTime;

void setup() {
  // put your setup code here, to run once:
  pinMode(sigPin, OUTPUT);
}

void loop() {
  // put your main code here, to run repeatedly:
  PORTD = B00001000;
  delayMicroseconds(onTime);
  PORTD = B00000000;
  delayMicroseconds(offTime);
}
