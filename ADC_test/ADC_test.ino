/*
  RTT ADC test
  Creates delayed square waves for use in ADC testing rig for RTT

  modified 29 Jun 2017
  by Sam O'Mahoony
*/

#define signalPin_1 3
#define signalPin_2 4
#define signalPin_3 5
#define signalPin_4 6

uint8_t signalPins[4] = {signalPin_1, signalPin_2, signalPin_3, signalPin_4};
uint8_t freqDelay = 200; // delay in ms

void setup() {
  for (int i = 0; i < sizeof(signalPins) / sizeof(signalPins[0]); i++) {
    pinMode(signalPins[i], OUTPUT);
  }
}

// the loop function runs over and over again forever
void loop() {
  for (int i = 0; i < sizeof(signalPins) / sizeof(signalPins[0]); i++) {
    digitalWrite(signalPins[i], HIGH);
    delay(freqDelay);
    digitalWrite(signalPins[i], LOW);
  }
}
