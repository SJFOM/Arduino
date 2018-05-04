// the sensor communicates using SPI, so include the library:
#include <SPI.h>
#include "xl362.h"
#include "xl362_io.h"

#define FIFO_MODE

// pins used for the connection with the sensor
// the other you need are controlled by the SPI library):
int const chipSelectPin = 3;

uint8_t device_id = 0;
int16_t x = 0;
int16_t y = 0;
int16_t z = 0;

#ifdef FIFO_MODE
int const fifoReadyPin = 9;
bool volatile fifoReady = false;
uint8_t fifoEntries_L = 0U;
uint8_t fifoEntries_H = 0U;
unsigned int fifoEntries  = 0U;
unsigned int const fifoNumSamples = 510U;
unsigned int const fifoNumBytesToRead = fifoNumSamples * 2U;
uint8_t volatile fifoBuffer[fifoNumBytesToRead];
#endif

char charBuf[30];

void setup() {
  Serial.begin(115200);

  // start the SPI library:
  SPI.begin();

  pinMode(chipSelectPin, OUTPUT);

  // delay to see the device ID output on the Serial monitor
  delay(500);

  Serial.println("Reading device id...");
  device_id = xl362Read(XL362_DEVID_AD, 1);
  Serial.print("Device ID: 0x");
  Serial.println(device_id, HEX);

  // Set ODR
  xl362Write(XL362_FILTER_CTL, XL362_RATE_25 | XL362_RANGE_2G);

#ifdef FIFO_MODE
  // Setup FIFO
  xl362Write(XL362_FIFO_CONTROL, ((fifoNumSamples > 0xff) ? XL362_FIFO_SAMPLES_AH : 0)
             | XL362_FIFO_MODE_FIFO);

  // Set up the FIFO sample register to wait for 510 bytes
  xl362Write(XL362_FIFO_SAMPLES, (fifoNumSamples & 0xff));


  // MAP INT1 to FIFO WATERMARK - i.e. set interrupt when fifo matches "fifoNumSamples"
  xl362Write(XL362_INTMAP1, XL362_INT_FIFO_WATERMARK);

  // setup interrupt pins
  pinMode(fifoReadyPin, INPUT_PULLDOWN);
  attachInterrupt(digitalPinToInterrupt(fifoReadyPin), ISR_fifoReady, RISING);
#endif

  // Set device to measurement mode
  xl362Write(XL362_POWER_CTL, XL362_MEASURE_3D);
}
void loop() {
#ifdef FIFO_MODE
  if (fifoReady == true) {
    fifoReady = false;
    (void)xl362FifoRead();
    for (uint16_t i = 0U; i < fifoNumBytesToRead; i += 6U) {
      x = ((int16_t)(((uint16_t)(fifoBuffer[i + 1U] << 12U)) + ((uint16_t)(fifoBuffer[i + 0U] << 4U)))) >> 4U;
      y = ((int16_t)(((uint16_t)(fifoBuffer[i + 3U] << 12U)) + ((uint16_t)(fifoBuffer[i + 2U] << 4U)))) >> 4U;
      z = ((int16_t)(((uint16_t)(fifoBuffer[i + 5U] << 12U)) + ((uint16_t)(fifoBuffer[i + 4U] << 4U)))) >> 4U;
      sprintf(charBuf, "x - %d\ty - %d\tz - %d", x, y, z);
      Serial.println(charBuf);
    }
  }
#else
  x = xl362Read(XL362_XDATAH, 1) * 256 + xl362Read(XL362_XDATAL, 1);
  y = xl362Read(XL362_YDATAH, 1) * 256 + xl362Read(XL362_YDATAL, 1);
  z = xl362Read(XL362_ZDATAH, 1) * 256 + xl362Read(XL362_ZDATAL, 1);
  //  x = xl362Read(XL362_XDATA8,1);
  //  y = xl362Read(XL362_YDATA8,1);
  //  z = xl362Read(XL362_ZDATA8,1);
  sprintf(charBuf, "x - %d\t\ty - %d\t\tz - %d", x, y, z);
  Serial.println(charBuf);
  delay(20);
#endif
}

#ifdef FIFO_MODE

void ISR_fifoReady(void) {
  fifoReady = true;
}


//Read from the ADXL362' FIFO:
void xl362FifoRead(void) {
  // take the chip select low to select the device:
  digitalWrite(chipSelectPin, LOW);

  // send the read register command
  SPI.transfer(XL362_FIFO_READ);

  // if you still have another byte to read:
  for (uint16_t i = 0; i <  fifoNumBytesToRead; i++) {
    fifoBuffer[i] = (uint8_t) SPI.transfer(0x00);
  }
  // take the chip select high to de-select:
  digitalWrite(chipSelectPin, HIGH);
}

#endif

//Read from a register on the ADXL362:
unsigned int xl362Read(byte regToRead, int bytesToRead ) {
  byte inByte = 0;           // incoming byte from the SPI
  unsigned int result = 0;   // result to return

  // take the chip select low to select the device:
  digitalWrite(chipSelectPin, LOW);

  // send the read register command
  SPI.transfer(XL362_REG_READ);
  // send the device the register you want to read:
  SPI.transfer(regToRead);
  // send a value of 0 to read the first byte returned:
  result = SPI.transfer(0x00);

  // decrement the number of bytes left to read:
  bytesToRead--;
  // if you still have another byte to read:
  if (bytesToRead > 0) {
    // shift the first byte left, then get the second byte:
    result = result << 8;
    inByte = SPI.transfer(0x00);
    // combine the byte you just got with the previous one:
    result = result | inByte;
    // decrement the number of bytes left to read:
    bytesToRead--;
  }

  // take the chip select high to de-select:
  digitalWrite(chipSelectPin, HIGH);
  // return the result:
  return (result);
}


//Sends a write command to ADXL362
void xl362Write(byte regToWrite, byte thisValue) {
  // take the chip select low to select the device:
  digitalWrite(chipSelectPin, LOW);

  SPI.transfer(XL362_REG_WRITE);  //Send register write command
  SPI.transfer(regToWrite);       //Send register location
  SPI.transfer(thisValue);        //Send value to record into register

  // take the chip select high to de-select:
  digitalWrite(chipSelectPin, HIGH);
}

