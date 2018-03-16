#include <LiquidCrystal595.h>
#include <avr/eeprom.h>

//we have to change prescaler for the ADC to make the conversion happen faster
//this code section was suggested on the arduino forum
#define FASTADC 1
// defines for setting and clearing register bits
#ifndef cbi
#define cbi(sfr, bit) (_SFR_BYTE(sfr) &= ~_BV(bit))
#endif
#ifndef sbi
#define sbi(sfr, bit) (_SFR_BYTE(sfr) |= _BV(bit))
#endif

//define the input and output pin we will use
#define DISCHARGE_PIN 7
#define ESR_PIN A0
#define PULSE_SMALL 8 //this is not used in the sketch, implement it as needed (for 5mA current needed for smaller cap value measurement)
#define PULSE_PIN 9
#define BUTTON_PIN 0

//function prototype
unsigned long measureESR(void);//measuring function, increases ADC to 16bit resolution through oversampling

//global variables
unsigned long esrSamples;
double miliVolt;
double esrVal;
double esrCal;
double vRef = 1.093;//voltage on the Vref pin (this sketch uses internal voltage reference 1.1V)
double current = 0.046200;//proper calibration can be done entering the right value for the current (U=I*R)
//idealy this is 0.05 A, this condition is fulfilled only if R10 is 100 Ohm, Vcc is exactly 5V and the transistor
//while fully saturated idealy is at 0 ohm.

//this is my display setup, I'm using Stephen Hobley's 3 wire setup and class
//exchange to what is good for you
LiquidCrystal595 lcd(6, 4, 2);

void setup(void)
{
  lcd.begin(20, 2);
  lcd.setLED2Pin(HIGH);
  lcd.setCursor(0, 0);
  lcd.print("ESR meter");
  lcd.setCursor(5, 1);
  lcd.print("version 0.1");
  delay(1000);

  lcd.print("Seting up...");

  analogReference(INTERNAL);//setting vRef to internal reference 1.1V

  pinMode(ESR_PIN, INPUT);//reading miliVolt
  pinMode(PULSE_PIN, OUTPUT);
  digitalWrite(PULSE_PIN, HIGH); //low enables T1
  pinMode(DISCHARGE_PIN, OUTPUT);
  digitalWrite(PULSE_PIN, HIGH); //low disables T2
  pinMode(BUTTON_PIN, INPUT); //setting up for a button (will use this for zeroing)
  digitalWrite(BUTTON_PIN, HIGH); //enabling the pull up on the button, when button pressed to the ground zeroes out the cable
  delay(1000);
  lcd.clear();
  lcd.print("Please Wait...");

  //seting prescaller to 32 for faster adc (500khz)
  //at 500khz  results are still looking good (same values as if 250khz ADC clock)
  // the shorter the pulse on a small value capacitor it has no time to charge and denaturate de result
  if (FASTADC) {
    sbi(ADCSRA, ADPS2);
    cbi(ADCSRA, ADPS1);
    sbi(ADCSRA, ADPS0);
  }

  //reading calibration value, it will be ok if already calibrated, else it might be bogus depends on the content of EEPROM
  //but will be ok after first calibration
  eeprom_read_block((void*)&esrCal, (void*)0, sizeof(esrCal));
}

void loop(void)
{
  lcd.setLED1Pin(HIGH);
  esrSamples = measureESR();//this function takes a while,)
  // so we don't need other delay for the lcd (this functions time gives the refresh rate for display
  miliVolt = (esrSamples * vRef) / 65.535;//calculating voltage on AIN0 pin
  esrVal = (miliVolt) / current - esrCal; //calculate ESR in miliOhm (pls read forum for correct formula)

  lcd.clear();
  lcd.print("  V:");
  lcd.print(miliVolt, 4);
  lcd.setCursor(13, 0);
  lcd.print("mV");
  lcd.setCursor(0, 1);
  lcd.print("ESR:");
  lcd.print(esrVal, 4);
  lcd.setCursor(13, 1);
  lcd.print("m");
  lcd.print((char)244);

  //for zeroing the cables, this can be quite a big resistance compared to the values we intend to measure
  //so it is a good idea to try to reduce in any way possible this influence (short cables, soldering the cables, etc)
  if (!digitalRead(BUTTON_PIN)) {
    lcd.clear();
    lcd.print("Zeroing...");
    esrCal = (miliVolt) / current;
    lcd.print(" done!");
    lcd.setCursor(0, 1);
    //writing calibration value into EEPROM so we don't have to calibrate on restart
    eeprom_write_block((const void*)&esrCal, (void*)0, sizeof(esrCal));
    lcd.print("saved to EEPROM");
    delay(400);
  }
}

//this is where the magic happens, it really works and gives some
//incredibly good results! if you need sub milivolt accuracy is a good way to go
//noise is good ;) if in doubt must read oversampling on ADC from AVR docs
unsigned long measureESR()
{
  unsigned long samples = 0;
  unsigned int acumulator = 0;
  int i = 0;
  //oversampling 4096 times (for 16 bit is 4^(desiredResolution - ADCresolution))
  while (i < 4096) {
    digitalWrite(DISCHARGE_PIN, LOW); //disable discharging
    digitalWrite(PULSE_PIN, LOW); //making a miliVolt pulse of 50mA
    delayMicroseconds(1);//on the scope it looks that after enabling the pulse a litle delay is
    //recomended so the oscillations fade away
    acumulator = analogRead(ESR_PIN);//reading value on AIN0
    digitalWrite(PULSE_PIN, HIGH); //stopping pulse
    digitalWrite(DISCHARGE_PIN, HIGH); //discharging the capacitors
    delayMicroseconds(600);//waiting a bit longer to fully discharge before another pulse
    samples += acumulator;//acumulating the readings
    i++;
  }
  //we have samples, let's go and compute value
  samples = samples >> 6;//decimating value
  return samples;//all done returning sampled value
}
