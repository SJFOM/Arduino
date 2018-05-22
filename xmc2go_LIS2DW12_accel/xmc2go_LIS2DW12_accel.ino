// the sensor communicates using SPI, so include the library:
#include <SPI.h>
#include "lis2dw12_reg.h"

#define BYTE_TO_BINARY_PATTERN "%c%c%c%c%c%c%c%c"
#define BYTE_TO_BINARY(byte)  \
  (byte & 0x80 ? '1' : '0'), \
  (byte & 0x40 ? '1' : '0'), \
  (byte & 0x20 ? '1' : '0'), \
  (byte & 0x10 ? '1' : '0'), \
  (byte & 0x08 ? '1' : '0'), \
  (byte & 0x04 ? '1' : '0'), \
  (byte & 0x02 ? '1' : '0'), \
  (byte & 0x01 ? '1' : '0')

#define FIFO_MODE

// pins used for the connection with the sensor
// the other you need are controlled by the SPI library:
int const lis2DchipSelectPin  = 5U;
int const adxlChipSelectPin   = 3U;

uint8_t device_id = 0;
int16_t x = 0;
int16_t y = 0;
int16_t z = 0;

// Config bytes for accelerometer
const uint8_t bduByte         = PROPERTY_ENABLE;
const uint8_t rangeByte       = LIS2DW12_8g;
const uint8_t filterByte      = LIS2DW12_LPF_ON_OUT;
const uint8_t filterBW        = LIS2DW12_ODR_DIV_2;
const uint8_t powerConfigByte = LIS2DW12_CONT_LOW_PWR_LOW_NOISE_2;
const uint8_t csPullupByte    = LIS2DW12_PULL_UP_CONNECT;
const uint8_t pinModeByte     = LIS2DW12_PUSH_PULL;
const uint8_t odrByte         = LIS2DW12_XL_ODR_25Hz;


// create a buffer for the acceleromter data
uint8_t volatile dataBuf[6U];
uint8_t dataRightShift = 2U;

#ifdef FIFO_MODE
// fifo pin config byte(s)
int const fifoReadyPin = 9U;

// fifo mode config bytes
const uint8_t fifoModeByte = LIS2DW12_STREAM_MODE;

// fifo data bytes
bool volatile fifoReady = false;
uint8_t fifoEntries_L = 0U;
uint8_t fifoEntries_H = 0U;
unsigned int fifoEntries  = 0U;
unsigned int const fifoNumSamples = 32U;
unsigned int const fifoNumBytesToRead = fifoNumSamples * 6U;
uint8_t volatile fifoBuffer[fifoNumBytesToRead];
#endif

char charBuf[30U];

void setup() {
  Serial.begin(115200);

  // start the SPI library:
  SPI.begin();

  // Ensure pin state defaults to high
  pinMode(lis2DchipSelectPin, OUTPUT);
  digitalWrite(lis2DchipSelectPin, HIGH);

  // configure adxl to be out of use here
  pinMode(adxlChipSelectPin, OUTPUT);
  digitalWrite(adxlChipSelectPin, HIGH);

  //  memset(dataBuf, 0, sizeof(dataBuf) / sizeof(dataBuf[0]));

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
     Union type struct for parsing register information
  */
  lis2dw12_reg_t reg;

  /*
      Enable Block Data Update
  */
  reg.byte = lis2dw12Read(LIS2DW12_CTRL2, 1);
  reg.ctrl2.bdu = bduByte;
  lis2dw12Write(LIS2DW12_CTRL2, (uint8_t) reg.byte);

  /*
     Set full scale
  */
  reg.byte = lis2dw12Read(LIS2DW12_CTRL6, 1);
  reg.ctrl6.fs = rangeByte;
  lis2dw12Write(LIS2DW12_CTRL6, (uint8_t) reg.byte);
  /*
     Configure filtering chain
  */
  /* Accelerometer - filter path */
  reg.byte = lis2dw12Read(LIS2DW12_CTRL6, 1);
  reg.ctrl6.fds = ( filterByte & 0x10 ) >> 4;
  lis2dw12Write(LIS2DW12_CTRL6, (uint8_t) reg.byte);

  reg.byte = lis2dw12Read(LIS2DW12_CTRL_REG7, 1);
  reg.ctrl_reg7.usr_off_on_out = filterByte & 0x01;
  lis2dw12Write(LIS2DW12_CTRL_REG7, (uint8_t) reg.byte);

  /* Accelerometer - bandwidth */
  reg.byte = lis2dw12Read(LIS2DW12_CTRL6, 1);
  reg.ctrl6.bw_filt = filterBW;
  lis2dw12Write(LIS2DW12_CTRL6, (uint8_t) reg.byte);

  /*
     Configure power mode
  */
  reg.byte = lis2dw12Read(LIS2DW12_CTRL1, 1);
  reg.ctrl1.mode = ( powerConfigByte & 0x0C ) >> 2;
  reg.ctrl1.lp_mode = powerConfigByte & 0x03;
  lis2dw12Write(LIS2DW12_CTRL1, (uint8_t) reg.byte);

  reg.byte = lis2dw12Read(LIS2DW12_CTRL6, 1);
  reg.ctrl6.low_noise = ( powerConfigByte & 0x10 ) >> 4;
  lis2dw12Write(LIS2DW12_CTRL6, (uint8_t) reg.byte);

  if (reg.ctrl1.lp_mode == 0) {
    // its in 12-bit resolution mode so need to adjust accordingly
    dataRightShift = 4U;
  }

  /*
     Set CS pull-up connected state
  */
  reg.byte = lis2dw12Read(LIS2DW12_CTRL2, 1);
  reg.ctrl2.cs_pu_disc = csPullupByte;
  lis2dw12Write(LIS2DW12_CTRL2, (uint8_t) reg.byte);

  /*
     Set pin mode - push-pull or open drain
  */
  reg.byte = lis2dw12Read(LIS2DW12_CTRL3, 1);
  reg.ctrl3.pp_od = pinModeByte;
  lis2dw12Write(LIS2DW12_CTRL3, (uint8_t) reg.byte);

#ifdef FIFO_MODE
  // Setup FIFO mode
  reg.byte = lis2dw12Read(LIS2DW12_FIFO_CTRL, 1);
  reg.fifo_ctrl.fmode = fifoModeByte;
  lis2dw12Write(LIS2DW12_FIFO_CTRL, (uint8_t) reg.byte);

  // MAP INT1 to FIFO full recognition mode
  reg.byte = lis2dw12Read(LIS2DW12_CTRL_REG7, 1);
  lis2dw12_ctrl4_int1_pad_ctrl_t int1CtrlStruct = {0};
  int1CtrlStruct.int1_diff5 = 1;

  if (int1CtrlStruct.int1_tap || int1CtrlStruct.int1_ff || int1CtrlStruct.int1_wu
      || int1CtrlStruct.int1_single_tap || int1CtrlStruct.int1_6d) {
    reg.ctrl_reg7.interrupts_enable = PROPERTY_ENABLE;
  }
  else {
    reg.ctrl_reg7.interrupts_enable = PROPERTY_DISABLE;
  }

  uint8_t int1CtrlByte = (uint8_t) (int1CtrlStruct.int1_6d << 7) |
                         (int1CtrlStruct.int1_single_tap << 6) |
                         (int1CtrlStruct.int1_wu         << 5) |
                         (int1CtrlStruct.int1_ff         << 4) |
                         (int1CtrlStruct.int1_tap        << 3) |
                         (int1CtrlStruct.int1_diff5      << 2) |
                         (int1CtrlStruct.int1_fth        << 1) |
                         (int1CtrlStruct.int1_drdy       << 0);

  lis2dw12Write(LIS2DW12_CTRL4_INT1_PAD_CTRL, (uint8_t) int1CtrlByte);
  lis2dw12Write(LIS2DW12_CTRL_REG7, (uint8_t) reg.byte);

  /*Reset FIFO address automatically
     From datasheet - page 29:
     In order to minimize communication between the master and slave, the address read may be
     automatically incremented by the device by setting the IF_ADD_INC bit of CTRL2 (21h) to
     '1'; the device rolls back to 0x28 when register 0x2D is reached.
  */
  reg.byte = lis2dw12Read(LIS2DW12_CTRL2, 1);
  reg.ctrl2.if_add_inc = 1;
  lis2dw12Write(LIS2DW12_CTRL2, (uint8_t) reg.byte);

  // setup interrupt pins
  pinMode(fifoReadyPin, INPUT_PULLDOWN);
  attachInterrupt(digitalPinToInterrupt(fifoReadyPin), ISR_fifoReady, RISING);
#endif

  /*
    Set Output Data Rate
  */
  reg.byte = lis2dw12Read(LIS2DW12_CTRL1, 1);
  reg.ctrl1.odr = odrByte;
  lis2dw12Write(LIS2DW12_CTRL1, (uint8_t) reg.byte);

  reg.byte = lis2dw12Read(LIS2DW12_CTRL3, 1);
  reg.ctrl3.slp_mode = ( odrByte & 0x30 ) >> 4;
  lis2dw12Write(LIS2DW12_CTRL3, (uint8_t) reg.byte);

}

void loop() {
#ifdef FIFO_MODE
  if (fifoReady == true) {
    fifoReady = false;
    (void)lis2dw12FifoRead();
    for (uint16_t i = 0U; i < fifoNumBytesToRead; i += 6U) {
      x = ((int16_t)(((uint16_t)(fifoBuffer[i + 1U] << 8U)) + fifoBuffer[i + 0U])) >> dataRightShift;
      y = ((int16_t)(((uint16_t)(fifoBuffer[i + 3U] << 8U)) + fifoBuffer[i + 2U])) >> dataRightShift;
      z = ((int16_t)(((uint16_t)(fifoBuffer[i + 5U] << 8U)) + fifoBuffer[i + 4U])) >> dataRightShift;
      //      sprintf(charBuf, "x - %d\ty - %d\tz - %d", x, y, z);
      sprintf(charBuf, "%d,%d,%d", x, y, z);
      Serial.println(charBuf);
    }
  }
#else

  /*
    Read output only if new value is available
  */

  lis2dw12_reg_t reg;

  reg.byte = lis2dw12Read(LIS2DW12_STATUS, 1U);

  if (reg.status.drdy == 1U) {
    lis2dw12Read(LIS2DW12_OUT_X_L, 6U);

    x = ((int16_t)((uint16_t)(dataBuf[1U] << 8U) + dataBuf[0U])) >> 2U;
    y = ((int16_t)((uint16_t)(dataBuf[3U] << 8U) + dataBuf[2U])) >> 2U;
    z = ((int16_t)((uint16_t)(dataBuf[5U] << 8U) + dataBuf[4U])) >> 2U;

    sprintf(charBuf, "x - %d\t\ty - %d\t\tz - %d", x, y, z);
    Serial.println(charBuf);
    //    sprintf(charBuf, "x  -  "BYTE_TO_BINARY_PATTERN" "BYTE_TO_BINARY_PATTERN" . %d",
    //            BYTE_TO_BINARY(x >> 8), BYTE_TO_BINARY(x), x);
    //    Serial.println(charBuf);
  }

#endif
}

#ifdef FIFO_MODE

void ISR_fifoReady(void) {
  fifoReady = true;
}


//Read from the LIS2DW12 FIFO:
void lis2dw12FifoRead(void) {
  // take the chip select low to select the device:
  digitalWrite(lis2DchipSelectPin, LOW);

  // send the read register command
  SPI.transfer(LIS2DW12_OUT_X_L | 0x80);

  // if you still have another byte to read:
  for (uint16_t i = 0; i <  fifoNumBytesToRead; i++) {
    fifoBuffer[i] = (uint8_t) SPI.transfer(0x00);
  }
  // take the chip select high to de-select:
  digitalWrite(lis2DchipSelectPin, HIGH);
}

#endif

//Read from a register on the LIS2DW12:
uint8_t lis2dw12Read(byte regToRead, int bytesToRead ) {
  byte inByte = 0;           // incoming byte from the SPI
  unsigned int result = 0;   // result to return

  // take the chip select low to select the device:
  digitalWrite(lis2DchipSelectPin, LOW);

  // send the device the register you want to read
  // Note: Must "OR" with MSB (0x80) to indicate READ operation
  SPI.transfer(regToRead | 0x80);

  for (uint16_t i = 0; i <  bytesToRead; i++) {
    dataBuf[i] = (uint8_t) SPI.transfer(0x00);
    //    Serial.print("dataBuf[");
    //    Serial.print(i); Serial.print("] - ");
    //    Serial.println(dataBuf[i]);
  }

  // take the chip select high to de-select:
  digitalWrite(lis2DchipSelectPin, HIGH);

  return dataBuf[0];
}


// Sends a write command to LIS2DW12
void lis2dw12Write(byte regToWrite, byte thisValue) {
  // take the chip select low to select the device:
  digitalWrite(lis2DchipSelectPin, LOW);

  SPI.transfer(regToWrite);       //Send register location
  SPI.transfer(thisValue);        //Send value to record into register

  // take the chip select high to de-select:
  digitalWrite(lis2DchipSelectPin, HIGH);
}

