// the sensor communicates using SPI, so include the library:
#include <SPI.h>
#include "lis2dw12_reg.h"

#define FIFO_MODE

// pins used for the connection with the sensor
// the other you need are controlled by the SPI library):
int const chipSelectPin = 5;
int const adxlChipSelectPin = 9;

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

  // Ensure pin state defaults to high
  pinMode(chipSelectPin, OUTPUT);
  digitalWrite(chipSelectPin, HIGH);

  // configure adxl to be out of use here
  pinMode(adxlChipSelectPin, OUTPUT);
  digitalWrite(adxlChipSelectPin, HIGH);

  // delay to see the device ID output on the Serial monitor
  delay(500);

  Serial.println("Reading device id...");
  device_id = lis2dw12Read(LIS2DW12_WHO_AM_I, 1);

  Serial.print("Device ID: 0x");
  Serial.println(device_id, HEX);

  if (device_id == LIS2DW12_ID) {
    Serial.println("Device found!");
  } else {
    Serial.println("Device not found yet...");
  }
  /*
   * Set Output Data Rate
   */
  lis2dw12_data_rate_set(&dev_ctx, LIS2DW12_XL_ODR_25Hz);
  
  //#ifdef FIFO_MODE
  //  // Setup FIFO
  //  lis2dw12Write(XL362_FIFO_CONTROL, ((fifoNumSamples > 0xff) ? XL362_FIFO_SAMPLES_AH : 0)
  //             | XL362_FIFO_MODE_FIFO);
  //
  //  // Set up the FIFO sample register to wait for 510 bytes
  //  lis2dw12Write(XL362_FIFO_SAMPLES, (fifoNumSamples & 0xff));
  //
  //
  //  // MAP INT1 to FIFO WATERMARK - i.e. set interrupt when fifo matches "fifoNumSamples"
  //  lis2dw12Write(XL362_INTMAP1, XL362_INT_FIFO_WATERMARK);
  //
  //  // setup interrupt pins
  //  pinMode(fifoReadyPin, INPUT_PULLDOWN);
  //  attachInterrupt(digitalPinToInterrupt(fifoReadyPin), ISR_fifoReady, RISING);
  //#endif
  //
  //  // Set device to measurement mode
  //  lis2dw12Write(XL362_POWER_CTL, XL362_MEASURE_3D);
}
void loop() {
  //#ifdef FIFO_MODE
  //  if (fifoReady == true) {
  //    fifoReady = false;
  //    (void)lis2dw12FifoRead();
  //    for (uint16_t i = 0U; i < fifoNumBytesToRead; i += 6U) {
  //      x = ((int16_t)(((uint16_t)(fifoBuffer[i + 1U] << 12U)) + ((uint16_t)(fifoBuffer[i + 0U] << 4U)))) >> 4U;
  //      y = ((int16_t)(((uint16_t)(fifoBuffer[i + 3U] << 12U)) + ((uint16_t)(fifoBuffer[i + 2U] << 4U)))) >> 4U;
  //      z = ((int16_t)(((uint16_t)(fifoBuffer[i + 5U] << 12U)) + ((uint16_t)(fifoBuffer[i + 4U] << 4U)))) >> 4U;
  //      sprintf(charBuf, "x - %d\ty - %d\tz - %d", x, y, z);
  //      Serial.println(charBuf);
  //    }
  //  }
  //#else
  //  x = lis2dw12Read(XL362_XDATAH, 1) * 256 + lis2dw12Read(XL362_XDATAL, 1);
  //  y = lis2dw12Read(XL362_YDATAH, 1) * 256 + lis2dw12Read(XL362_YDATAL, 1);
  //  z = lis2dw12Read(XL362_ZDATAH, 1) * 256 + lis2dw12Read(XL362_ZDATAL, 1);
  //  //  x = lis2dw12Read(XL362_XDATA8,1);
  //  //  y = lis2dw12Read(XL362_YDATA8,1);
  //  //  z = lis2dw12Read(XL362_ZDATA8,1);
  //  sprintf(charBuf, "x - %d\t\ty - %d\t\tz - %d", x, y, z);
  //  Serial.println(charBuf);
  //  delay(20);
  //#endif
}

//#ifdef FIFO_MODE
//
//void ISR_fifoReady(void) {
//  fifoReady = true;
//}
//
//
////Read from the LIS2DW12 FIFO:
//void lis2dw12FifoRead(void) {
//  // take the chip select low to select the device:
//  digitalWrite(chipSelectPin, LOW);
//
//  // send the read register command
//  SPI.transfer(XL362_FIFO_READ);
//
//  // if you still have another byte to read:
//  for (uint16_t i = 0; i <  fifoNumBytesToRead; i++) {
//    fifoBuffer[i] = (uint8_t) SPI.transfer(0x00);
//  }
//  // take the chip select high to de-select:
//  digitalWrite(chipSelectPin, HIGH);
//}
//
//#endif

//Read from a register on the LIS2DW12:
unsigned int lis2dw12Read(byte regToRead, int bytesToRead ) {
  byte inByte = 0;           // incoming byte from the SPI
  unsigned int result = 0;   // result to return

  // take the chip select low to select the device:
  digitalWrite(chipSelectPin, LOW);

  // send the device the register you want to read
  // Note: Must "OR" with MSB (0x80) to indicate READ operation
  SPI.transfer(regToRead | 0x80);
  // send a value of 0 to read the first byte returned:
  result = SPI.transfer(0x00);

  //decrement the number of bytes left to read:
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


//Sends a write command to LIS2DW12
//void lis2dw12Write(byte regToWrite, byte thisValue) {
//  // take the chip select low to select the device:
//  digitalWrite(chipSelectPin, LOW);
//
//  SPI.transfer(XL362_REG_WRITE);  //Send register write command
//  SPI.transfer(regToWrite);       //Send register location
//  SPI.transfer(thisValue);        //Send value to record into register
//
//  // take the chip select high to de-select:
//  digitalWrite(chipSelectPin, HIGH);
//}

