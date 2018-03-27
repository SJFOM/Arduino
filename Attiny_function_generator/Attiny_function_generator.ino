/* Tiny Function Generator with Sine Wave

   David Johnson-Davies - www.technoblogy.com - 7th March 2018
   ATtiny85 @ 8 MHz (internal PLL; BOD disabled)

   CC BY 4.0
   Licensed under a Creative Commons Attribution 4.0 International license:
   http://creativecommons.org/licenses/by/4.0/
*/

#include <Wire.h>
#define NOINIT __attribute__ ((section (".noinit")))

// Don't initialise these on reset
int Wave NOINIT;
unsigned int Freq NOINIT;
int8_t Sinewave[256] NOINIT;

typedef void (*wavefun_t)();

// Direct Digital Synthesis **********************************************

volatile unsigned int Acc, Jump;
volatile signed int X, Y;

void SetupDDS () {
  // Enable 64 MHz PLL and use as source for Timer1
  PLLCSR = 1 << PCKE | 1 << PLLE;

  // Set up Timer/Counter1 for PWM output
  TIMSK = 0;                               // Timer interrupts OFF

  /*
  	=============================================
	*  TCCR1 - Timer/Counter1 Control Register  *
	=============================================
	The PWM duty cycle is controlled by the OCR1A register with the 
	period of a PWM waveform being controlled by the OCR1C register.
	Upon counting up to the value of OCR1C (default 255), the Timer1 counter
	value is reset to zero.

	Here, since Timer1 is (over)clocked to 64MHz, the OCRC1 register gives a PWM
	waveform period of: 8MHz/(255+1) = 250kHz

	=======================
	*  TIMER1 PWM SIGNAL  *
	=======================

    Timer1 counter = 0	  
              |              Timer1 counter = OCR1C -> 0
              |                         |
      				|						              |
      				|						              v
      				|   OCR1A			          OCR1C
      				|	    |					          |
      			 	v_____v  				          v____v
      				|	    |					          |	   |
      				|	    |					          |	   |
      				|	    |					          |	   |
	. . . ------|	    |-------------------|	   |------ . . . . 
				      |<-----	   250kHz   ----->|

  */

  TCCR1 = 1 << PWM1A	// PWM A - Pulse Width Modulator A Enable
  		| 2 << COM1A0 	// Clear the OC1A output line on match
  		| 1 << CS10;	// 1:1 prescaler for 64MHz clock

  pinMode(1, OUTPUT);                      // Enable PWM output pin


  /*
  	=============================================
	*  TCCR0 - Timer/Counter0 Control Register  *
	=============================================
	Timer/Counter0 is operates off the default 8MHz system clock.
	Unlike the single register used for Timer/Counter1 (TCCR1), Timer/Counter0
	uses two registers (TCCR0A and TCCR0B) to control Timer/Counter0.

	So, Timer/Counter0 is actually one timer output with the sole function of
	scheduling when to output the generated PWM samples.

	
	=========================================
	*  WGM - Waveform Generation Mode bits  *
	=========================================
	3 << WGM00: sets bits WGM00 and WGM01, which (when combined with WGM02 from TCCR0B below) enables Fast PWM mode.
	This sets the WGM into Mode 7 which is: Fast PWM - count up to OCR0A before reset

	NOTE: Clear Timer on Compare Match (CTC) Mode could also have been used here but "Fast PWM" 
	offers a very fast timer control compare operation which is "probably" better suited to this application.


	=======================
	*  TIMER0 PWM SIGNAL  *
	=======================

      	Timer0 counter = 0
      				|
      				|	    Timer0 counter = 0CR0A-> 0
      				|			      |
      				|			      |		    Timer0 counter = 0CR0A-> 0
      				|		 	      v			      |
      				|			    OCR0A			    v	      Timer0 counter = 0CR0A-> 0 ...			
      				| 			    |			    OCR0A		      |
      				|			      |			      |			    OCR0A ...
      				|		 	      |			      |			      |
      			 	v___________v  			    v___________v
      				|		 	      |			      |			      |
      				|		 	      |			      |			      |
      				|		 	      |      			|			      |
	. . . ------|		 	      |-----------|           |---- . . . . 
				      |<-16.4kHz->|<-16.4kHz->|<-16.4kHz->|

	NOTE: 8MHz / 8 / 61 ~= 16,393Hz -> 16.4kHz - the counter frequency and resulting interrupt "rate"
	NOTE: Each 'v' here represents an interrupt flag.


	=====================
	*  Output accuracy  *
	=====================
	By setting our output compare register OCR0A to 60 (60+1), we can get a very reasonable frequency output 
	resolution for our needs - "within 0.1%"

	Description:
	To achieve an output frequency of 1Hz, we set a "Jump" value to 4 where:
		16 bit max/4 = 65,536/4 = 16,384 counts.

	Since our sampling clock just exceeds this, we are able to output our samples to within a very close tolerance:
		16,384 Hz / 16,393 Hz ~= 0.99945 -> (1 - 0.99945)*100% = 0.05% ~= 0.1%

	Hence, as long as we set a "Jump" value to be a multiple of 4, we will be able to maintain this resolution 
	throughout the available frequency range.

  */

  // Set up Timer/Counter0 for 16.4kHz interrupt to output samples.
  TCCR0A = 3 << WGM00;                     // Fast PWM
  TCCR0B = 1 << WGM02 | 2 << CS00;         // 1/8 prescale
  TIMSK = 1 << OCIE0A;                     // Enable compare match, disable overflow
  OCR0A = 60;                              // Divide by 61
}

// Calculate sine wave
void CalculateSine () {
  int X = 0, Y = 8180;
  for (int i = 0; i < 256; i++) {
    X = X + (Y * 4) / 163;
    Y = Y - (X * 4) / 163;
    Sinewave[i] = X >> 6;
  }
}

void Sine () {
  Acc = Acc + Jump;
  OCR1A = Sinewave[Acc >> 8] + 128;
}

void Sawtooth () {
  Acc = Acc + Jump;
  OCR1A = Acc >> 8;
}

void Square () {
  Acc = Acc + Jump;
  int8_t temp = Acc >> 8;
  OCR1A = temp >> 7;
}

void Rectangle () {
  Acc = Acc + Jump;
  int8_t temp = Acc >> 8;
  temp = temp & temp << 1;
  OCR1A = temp >> 7;
}

void Triangle () {
  int8_t temp, mask;
  Acc = Acc + Jump;
  temp = Acc >> 8;
  mask = temp >> 7;
  temp = temp ^ mask;
  OCR1A = temp << 1;
}

void Chainsaw () {
  int8_t temp, mask, top;
  Acc = Acc + Jump;
  temp = Acc >> 8;
  mask = temp >> 7;
  top = temp & 0x80;
  temp = (temp ^ mask) | top;
  OCR1A = temp;
}

void Pulse () {
  Acc = Acc + Jump;
  int8_t temp = Acc >> 8;
  temp = temp & temp << 1 & temp << 2 & temp << 3;
  OCR1A = temp >> 7;
}

void Noise () {
  int8_t temp = Acc & 1;
  Acc = Acc >> 1;
  if (temp == 0) Acc = Acc ^ 0xB400;
  OCR1A = Acc;
}

const int nWaves = 8;
wavefun_t Waves[nWaves] = {Sine, Triangle, Sawtooth, Square, Rectangle, Pulse, Chainsaw, Noise};
wavefun_t Wavefun;

ISR(TIMER0_COMPA_vect) {
  Wavefun();
}

// OLED I2C 128 x 32 monochrome display **********************************************

const int OLEDAddress = 0x3C;

// Initialisation sequence for OLED module
int const InitLen = 24;
const unsigned char Init[InitLen] PROGMEM = {
  0xAE, // Display off
  0xD5, // Set display clock
  0x80, // Recommended value
  0xA8, // Set multiplex
  0x1F,
  0xD3, // Set display offset
  0x00,
  0x40, // Zero start line
  0x8D, // Charge pump
  0x14,
  0x20, // Memory mode
  0x01, // Vertical addressing
  0xA1, // 0xA0/0xA1 flip horizontally
  0xC8, // 0xC0/0xC8 flip vertically
  0xDA, // Set comp ins
  0x02,
  0x81, // Set contrast
  0x7F, // 0x00 to 0xFF
  0xD9, // Set pre charge
  0xF1,
  0xDB, // Set vcom detect
  0x40,
  0xA6, // Normal (0xA7=Inverse)
  0xAF  // Display on
};

const int data = 0x40;
const int single = 0x80;
const int command = 0x00;

void InitDisplay () {
  Wire.beginTransmission(OLEDAddress);
  Wire.write(command);
  for (uint8_t c = 0; c < InitLen; c++) Wire.write(pgm_read_byte(&Init[c]));
  Wire.endTransmission();
}

// Graphics **********************************************

int Scale = 2; // 2 for big characters
const int Space = 10;
const int Hz    = 11;
const int Icons = 13;

// Character set for digits, "Hz", and waveform icons - stored in program memory
const uint8_t CharMap[][6] PROGMEM = {
  { 0x3E, 0x51, 0x49, 0x45, 0x3E, 0x00 }, // 30
  { 0x00, 0x42, 0x7F, 0x40, 0x00, 0x00 },
  { 0x72, 0x49, 0x49, 0x49, 0x46, 0x00 },
  { 0x21, 0x41, 0x49, 0x4D, 0x33, 0x00 },
  { 0x18, 0x14, 0x12, 0x7F, 0x10, 0x00 },
  { 0x27, 0x45, 0x45, 0x45, 0x39, 0x00 },
  { 0x3C, 0x4A, 0x49, 0x49, 0x31, 0x00 },
  { 0x41, 0x21, 0x11, 0x09, 0x07, 0x00 },
  { 0x36, 0x49, 0x49, 0x49, 0x36, 0x00 },
  { 0x46, 0x49, 0x49, 0x29, 0x1E, 0x00 },
  { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }, // Space
  { 0x7F, 0x08, 0x08, 0x08, 0x7F, 0x00 }, // H
  { 0x44, 0x64, 0x54, 0x4C, 0x44, 0x00 }, // z
  { 0x0C, 0x02, 0x01, 0x01, 0x02, 0x3C }, // Sine
  { 0x40, 0x80, 0x80, 0x40, 0x30, 0x00 },
  { 0x08, 0x04, 0x02, 0x01, 0x02, 0x04 }, // Triangle
  { 0x08, 0x10, 0x20, 0x40, 0x20, 0x10 },
  { 0x40, 0x20, 0x10, 0x08, 0x04, 0x7E }, // Sawtooth
  { 0x40, 0x20, 0x10, 0x08, 0x04, 0x7E },
  { 0x7E, 0x02, 0x02, 0x02, 0x02, 0x7E }, // Square
  { 0x40, 0x40, 0x40, 0x40, 0x7E, 0x00 },
  { 0x7E, 0x02, 0x02, 0x7E, 0x40, 0x40 }, // Rectangle
  { 0x40, 0x40, 0x40, 0x40, 0x7E, 0x00 },
  { 0x7E, 0x40, 0x40, 0x40, 0x40, 0x40 }, // Pulse
  { 0x40, 0x40, 0x40, 0x40, 0x7E, 0x00 },
  { 0x78, 0x04, 0x02, 0x0F, 0x10, 0x20 }, // Chainsaw
  { 0x78, 0x04, 0x02, 0x0F, 0x10, 0x20 },
  { 0x0C, 0x78, 0x1E, 0x18, 0x70, 0x1F }, // Noise
  { 0x7C, 0x1C, 0x60, 0x38, 0x3E, 0x08 },
};

void ClearDisplay () {
  Wire.beginTransmission(OLEDAddress);
  Wire.write(command);
  // Set column address range
  Wire.write(0x21); Wire.write(0); Wire.write(127);
  // Set page address range
  Wire.write(0x22); Wire.write(0); Wire.write(3);
  Wire.endTransmission();
  // Write the data in 16 32-byte transmissions
  for (int i = 0 ; i < 32; i++) {
    Wire.beginTransmission(OLEDAddress);
    Wire.write(data);
    for (int i = 0 ; i < 32; i++) Wire.write(0);
    Wire.endTransmission();
  }
}

// Converts bit pattern abcdefgh into aabbccddeeffgghh
int Stretch (int x) {
  x = (x & 0xF0) << 4 | (x & 0x0F);
  x = (x << 2 | x) & 0x3333;
  x = (x << 1 | x) & 0x5555;
  return x | x << 1;
}

// Plots a character; line = 0 to 2; column = 0 to 21
void PlotChar(int c, int line, int column) {
  Wire.beginTransmission(OLEDAddress);
  Wire.write(command);
  // Set column address range
  Wire.write(0x21); Wire.write(column * 6); Wire.write(column * 6 + Scale * 6 - 1);
  // Set page address range
  Wire.write(0x22); Wire.write(line); Wire.write(line + Scale - 1);
  Wire.endTransmission();
  Wire.beginTransmission(OLEDAddress);
  Wire.write(data);
  for (uint8_t col = 0 ; col < 6; col++) {
    int bits = pgm_read_byte(&CharMap[c][col]);
    if (Scale == 1) Wire.write(bits);
    else {
      bits = Stretch(bits);
      for (int i = 2; i--;) {
        Wire.write(bits);
        Wire.write(bits >> 8);
      }
    }
  }
  Wire.endTransmission();
}

uint8_t DigitChar (unsigned int number, unsigned int divisor) {
  return (number / divisor) % 10;
}

// Display waveform icon
void PlotIcon (int wave, int line, int column) {
  PlotChar(Icons + 2 * wave, line, column); column = column + Scale;
  PlotChar(Icons + 2 * wave + 1, line, column);
}

// Display a 5-digit frequency starting at line, column
void PlotFreq (unsigned int freq, int line, int column) {
  boolean dig = false;
  for (unsigned int d = 10000; d > 0; d = d / 10) {
    char c = DigitChar(freq, d);
    if (c == 0 && !dig) c = Space; else dig = true;
    PlotChar(c, line, column);
    column = column + Scale;
  }
  PlotChar(Hz, line, column); column = column + Scale;
  PlotChar(Hz + 1, line, column);
}

// Rotary encoder **********************************************

const int EncoderA = 3;
const int EncoderB = 4;
const int MinFreq = 1;        // Hz
const int MaxFreq = 5000;     // Hz

volatile int a0;
volatile int c0;
volatile int Count = 0;

void SetupRotaryEncoder () {
  pinMode(EncoderA, INPUT_PULLUP);
  pinMode(EncoderB, INPUT_PULLUP);
  PCMSK = 1 << EncoderA;      // Configure pin change interrupt on A
  GIMSK = 1 << PCIE;          // Enable interrupt
  GIFR = 1 << PCIF;           // Clear interrupt flag
}

// Called when encoder value changes
void ChangeValue (bool Up) {
  int step = 1;
  if (Freq >= 1000) step = 100;
  else if (Freq >= 100) step = 10;
  Freq = max(min((Freq + (Up ? step : -step)), MaxFreq), MinFreq);
  PlotFreq(Freq, 1, 7);
  Jump = Freq * 4;
}

// Pin change interrupt service routine
ISR (PCINT0_vect) {
  int a = PINB >> EncoderA & 1;
  int b = PINB >> EncoderB & 1;
  if (a != a0) {              // A changed
    a0 = a;
    if (b != c0) {
      c0 = b;
      ChangeValue(a == b);
    }
  }
}

// Setup **********************************************

void setup() {
  Wire.begin();
  // Is it a power-on reset?
  if (MCUSR & 1) {
    Wave = 0; Freq = 100;     // Start with 100Hz Sine
    CalculateSine();
    InitDisplay();
    ClearDisplay();
  }
  else Wave = (Wave + 1) % nWaves;
  Wavefun = Waves[Wave];
  MCUSR = 0;
  SetupDDS();
  SetupRotaryEncoder();
  Jump = Freq * 4;
  PlotFreq(Freq, 1, 7);
  PlotIcon(Wave, 1, 0);
}

// Everything done by interrupts
void loop() {
}
