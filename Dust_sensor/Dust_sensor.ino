// Paul's GP2Y1010AU0F Dust Sensor Sketch

/*
  Arduino Uno (ATMEGA328) Port Allocations:

  A1:     A/D:                     Dust Sensor Analog Signal
  D2:     INT0 I/P:                Dust Sensor Interrupt - Link to D9
  D9:     Timer 1 OC1A PWM O/P:    Dust Sensor Samples - Link to D2
  D10:    Timer 1 OC1B PWM O/P:    Dust Sensor LED Pulses
*/

#define VOLTAGE 5.0 // Arduino supply voltage

// Download the Timer 1 Library (r11) from:
// http://playground.arduino.cc/Code/Timer1
// https://code.google.com/p/arduino-timerone/downloads/list

#include <TimerOne.h>

/*
   Smoothing

   Reads repeatedly from an analog input, calculating a running average.
   Keeps readings in an array and continually averages them.

   Created 22 April 2007
   By David A. Mellis  <dam@mellis.org>
   modified 9 Apr 2012
   by Tom Igoe
   http://www.arduino.cc/en/Tutorial/Smoothing

   This example code is in the public domain.
*/

/* With thanks to Adafruit for the millis code - plagiarised from the Adafruit GPS Library */

const int numReadings = 100;    // samples are taken at 100Hz so calculate average over 1sec

int readings[numReadings];      // the readings from the analog input
int readIndex = 0;              // the index of the current reading
long int total = 0;             // the running total
int latest_reading = 0;         // the latest reading
int average_reading = 0;        // the average reading

long int timeBetweenReads = 100;

int dustPin = A1;

// Initialisation routine

void setup()
{
  Serial.begin(9600);

  // Configure PWM Dust Sensor Sample pin (OC1A)
  pinMode(9, OUTPUT);
  // Configure PWM Dust Sensor LED pin (OC1B)
  pinMode(10, OUTPUT);
  // Configure INT0 to receive Dust Sensor Samples
  pinMode(2, INPUT_PULLUP);

  // Put Timer 1 into 16-bit mode to generate the 0.32ms low LED pulses every 10ms
  // A0 needs to be sampled 0.28ms after the falling edge of the LED pulse - via INT0 driven by OC1A (D9)
  Timer1.initialize(10000); // Set a timer of length 10000 microseconds (or 10ms - or 100Hz)
  Timer1.pwm(10, 991); // Set active high PWM of (10 - 0.32) * 1024 = 991
  Timer1.pwm(9, 999); // Set active high PWM of (10 - 0.28) * 1024 = 995 BUT requires a fiddle factor making it 999

  // Attach the INT0 interrupt service routine
  attachInterrupt(0, takeReading, RISING); // Sample A0 on the rising edge of OC1A

  // Initialise sample buffer
  for (int thisReading = 0; thisReading < numReadings; thisReading++) {
    readings[thisReading] = 0;
  }
}

// Dust sample interrupt service routine
void takeReading() {
  // subtract the last reading:
  total = total - readings[readIndex];
  // read from the sensor:
  latest_reading = analogRead(dustPin);
  readings[readIndex] = latest_reading;
  // add the reading to the total:
  total = total + latest_reading;
  // advance to the next position in the array:
  readIndex++;

  // if we're at the end of the array...wrap around to the beginning:
  if (readIndex >= numReadings) readIndex = 0;

  // calculate the average:
  average_reading = total / numReadings; // Seems to work OK with integer maths - but total does need to be long int
}

// Main loop

uint32_t timer = millis();

void loop()                     // run over and over again
{
  // if millis() or timer wraps around, we'll just reset it
  if (timer > millis())  timer = millis();

  // approximately every second or so, print out the dust reading
  if (millis() - timer > timeBetweenReads) {
    timer = millis(); // reset the timer

    float latest_dust = latest_reading * (VOLTAGE / 1023.0);
    float average_dust = average_reading * (VOLTAGE / 1023.0);
    float dust_ug_m3 = 0.0f;
    float dust_ug_m3_avg = 0.0f;

    if (latest_dust < 0.9) {
      dust_ug_m3 = 0.172 * latest_dust - 0.0999;
      dust_ug_m3_avg = 0.172 * average_dust - 0.0999;
    } else if (latest_dust <= 3.61) {
      dust_ug_m3 = 18.512 * pow(latest_dust, 6) - 220.46 * pow(latest_dust, 5)
                   + 1044.9 * pow(latest_dust, 4) - 2501.1 * pow(latest_dust, 3) +
                   3143.6 * pow(latest_dust, 2) - 1753.8 * latest_dust + 293.37;

      dust_ug_m3_avg = 18.512 * pow(average_dust, 6) - 220.46 * pow(average_dust, 5)
                       + 1044.9 * pow(average_dust, 4) - 2501.1 * pow(average_dust, 3) +
                       3143.6 * pow(average_dust, 2) - 1753.8 * average_dust + 293.37;
    } else {
      dust_ug_m3 = 2500.3 * latest_dust - 8503.2;
      dust_ug_m3_avg = 2500.3 * average_dust - 8503.2;
    }

    dust_ug_m3 = (dust_ug_m3 < 0) ? 0.0f : dust_ug_m3;
    dust_ug_m3_avg = (dust_ug_m3_avg < 0) ? 0.0f : dust_ug_m3_avg;

    //    Serial.print("Latest Dust Reading: ");
    //    Serial.print(latest_dust);
    //    Serial.print("Average Dust Reading: ");
    //    Serial.print(average_dust);
    //    Serial.print(" V  -  ");
    Serial.println(dust_ug_m3);
    //    Serial.print(dust_ug_m3_avg);
    //    Serial.println(" ug/m^3");
    //    Serial.print("\tAverage Dust Reading (V): ");
    //    Serial.println(average_dust);
  }
}
