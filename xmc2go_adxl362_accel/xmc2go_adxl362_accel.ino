// the sensor communicates using SPI, so include the library:
#include <SPI.h>
#include "xl362.h"
#include "xl362_io.h"

#define FIFO_MODE

// pins used for the connection with the sensor
// the other you need are controlled by the SPI library):
int const lis2DchipSelectPin  = 5U;
int const adxlChipSelectPin   = 3U;

uint8_t device_id = 0U;
int16_t x = 0;
int16_t y = 0;
int16_t z = 0;

// Config bytes for accelerometer
const uint8_t rangeByte       = XL362_RANGE_8G;
const uint8_t odrByte         = XL362_RATE_25;
const uint8_t powerConfigByte = (XL362_MEASURE_3D | XL362_LOW_POWER);

#ifdef FIFO_MODE
int const fifoReadyPin = 9U;
bool volatile fifoReady = false;
uint8_t fifoEntries_L = 0U;
uint8_t fifoEntries_H = 0U;
unsigned int fifoEntries  = 0U;
unsigned int const fifoNumSamples = 510U;
unsigned int const fifoNumBytesToRead = fifoNumSamples * 2U;
uint8_t volatile fifoBuffer[fifoNumBytesToRead];
#endif

char charBuf[30U];

void setup() {
  Serial.begin(115200U);

  // start the SPI library:
  SPI.begin();

  // Ensure pin state defaults to high
  pinMode(adxlChipSelectPin, OUTPUT);
  digitalWrite(adxlChipSelectPin, HIGH);

  // configure lis2d to be out of use here
  pinMode(lis2DchipSelectPin, OUTPUT);
  digitalWrite(lis2DchipSelectPin, HIGH);

  // delay to see the device ID output on the Serial monitor
  delay(500U);

  Serial.println("Reading device id...");
  device_id = xl362Read(XL362_DEVID_AD, 1U);
  Serial.print("Device ID: 0x");
  Serial.println(device_id, HEX);

  uint8_t regByte = 0U;

  // soft reset for safety
  regByte = XL362_SOFT_RESET_KEY;
  xl362Write(XL362_SOFT_RESET, regByte);
  delay(10); /* wait for soft reset to pass */
  
  // Set ODR
  regByte = xl362Read(XL362_FILTER_CTL, 1U);
  regByte |= (odrByte | rangeByte);
  xl362Write(XL362_FILTER_CTL, regByte);

#ifdef FIFO_MODE
  // Setup FIFO
  regByte = xl362Read(XL362_FIFO_CONTROL, 1U);
  regByte |= (((fifoNumSamples > 0xff) ? XL362_FIFO_SAMPLES_AH : 0U)
              | XL362_FIFO_MODE_FIFO);
  xl362Write(XL362_FIFO_CONTROL, regByte);

  // Set up the FIFO sample register to wait for 510 bytes
  regByte = xl362Read(XL362_FIFO_SAMPLES, 1U);
  regByte |= (fifoNumSamples & 0xff);
  xl362Write(XL362_FIFO_SAMPLES, regByte);


  // MAP INT1 to FIFO WATERMARK - i.e. set interrupt when fifo matches "fifoNumSamples"
  regByte = xl362Read(XL362_INTMAP1, 1U);
  regByte |= XL362_INT_FIFO_WATERMARK;
  xl362Write(XL362_INTMAP1, regByte);

  // setup interrupt pins
  pinMode(fifoReadyPin, INPUT_PULLDOWN);
  attachInterrupt(digitalPinToInterrupt(fifoReadyPin), ISR_fifoReady, RISING);
#endif

  // Set device to measurement mode
  regByte = xl362Read(XL362_POWER_CTL, 1U);
  regByte |= (powerConfigByte);
  xl362Write(XL362_POWER_CTL, regByte);
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
      //      sprintf(charBuf, "x - %d\ty - %d\tz - %d", x, y, z);
      sprintf(charBuf, "%d,%d,%d", x, y, z);
      Serial.println(charBuf);
    }
  }
#else
  x = xl362Read(XL362_XDATAH, 1U) * 256U + xl362Read(XL362_XDATAL, 1U);
  y = xl362Read(XL362_YDATAH, 1U) * 256U + xl362Read(XL362_YDATAL, 1U);
  z = xl362Read(XL362_ZDATAH, 1U) * 256U + xl362Read(XL362_ZDATAL, 1U);
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
  digitalWrite(adxlChipSelectPin, LOW);

  // send the read register command
  SPI.transfer(XL362_FIFO_READ);

  // if you still have another byte to read:
  for (uint16_t i = 0; i <  fifoNumBytesToRead; i++) {
    fifoBuffer[i] = (uint8_t) SPI.transfer(0x00);
  }
  // take the chip select high to de-select:
  digitalWrite(adxlChipSelectPin, HIGH);
}

#endif

//Read from a register on the ADXL362:
unsigned int xl362Read(byte regToRead, int bytesToRead ) {
  byte inByte = 0;           // incoming byte from the SPI
  unsigned int result = 0U;   // result to return

  // take the chip select low to select the device:
  digitalWrite(adxlChipSelectPin, LOW);

  // send the read register command
  SPI.transfer(XL362_REG_READ);
  // send the device the register you want to read:
  SPI.transfer(regToRead);
  // send a value of 0 to read the first byte returned:
  result = SPI.transfer(0x00);

  // decrement the number of bytes left to read:
  bytesToRead--;
  // if you still have another byte to read:
  if (bytesToRead > 0U) {
    // shift the first byte left, then get the second byte:
    result = result << 8U;
    inByte = SPI.transfer(0x00);
    // combine the byte you just got with the previous one:
    result = result | inByte;
    // decrement the number of bytes left to read:
    bytesToRead--;
  }

  // take the chip select high to de-select:
  digitalWrite(adxlChipSelectPin, HIGH);
  // return the result:
  return (result);
}


//Sends a write command to ADXL362
void xl362Write(byte regToWrite, byte thisValue) {
  // take the chip select low to select the device:
  digitalWrite(adxlChipSelectPin, LOW);

  SPI.transfer(XL362_REG_WRITE);  //Send register write command
  SPI.transfer(regToWrite);       //Send register location
  SPI.transfer(thisValue);        //Send value to record into register

  // take the chip select high to de-select:
  digitalWrite(adxlChipSelectPin, HIGH);
}

