/*
Wishy Washy
V: 0.2
*/

/* Comment this out to disable prints and save space */
#define BLYNK_PRINT Serial

#define PIN_UPTIME      V0
#define PIN_TEMPERATURE V1
#define PIN_ACCEL       V2
#define PIN_LCD         V3

#include <ESP8266WiFi.h>
#include <BlynkSimpleEsp8266.h>

#include<Wire.h>

#define DEBUG 0

const int MPU_addr = 0x68; // I2C address of the MPU-6050
int16_t accelX, accelY, accelZ, temperature;
const float MPU_ACCL_2_SCALE = 16384.0;
const float MPU_ACCL_4_SCALE = 8192.0;
const float MPU_ACCL_8_SCALE = 4096.0f;
const float MPU_ACCL_16_SCALE = 2048.0;

const uint8_t ACCEL_SCALE_2G = 0x00;
const uint8_t ACCEL_SCALE_4G = 0x01;
const uint8_t ACCEL_SCALE_8G = 0x10;
const uint8_t ACCEL_SCALE_16G = 0x11;

// initial accelerometer values
float accelX_init = 0.0f;
float accelY_init = 0.0f;
float accelZ_init = 0.0f;

// Blynk value update/push speedz
const long int timeoutValue = 1000L;

// Parameters for wash cycle:
const long int  TIME_WINDOW      = 3000L; // 3 seconds
const long int  TIME_UNTIL_ON    = 30000L; // time until machine kicks back into life between states
const long int  TIME_UNTIL_DONE  = 90000L;
const float     THRESHOLD        = 0.5f; // acceleration force threshold to indicate "ON" state

enum washState
{
  WASH_IDLE = 0,
  WASH_MAYBE_ON,
  WASH_ON,
  WASH_MAYBE_DONE,
  WASH_DONE
};

char *stateStrings[5] = {
  "WASH_IDLE",
  "WASH_MAYBE_ON",
  "WASH_ON",
  "WASH_MAYBE_DONE",
  "WASH_DONE"
};

struct Wash
{
  float threshold;
  washState prevState;
  washState curState;
  long int lastActiveTime;
  long int lastIdleTime;
  bool wasRecentlyActive;
} sWash;

struct Wash myWash;

// You should get Auth Token in the Blynk App.
// Go to the Project Settings (nut icon).
char auth[] = "67f7dc4a21a64f1e81a4a7087572cf2f";

// Your WiFi credentials.
// Set password to "" for open networks.
char ssid[] = "Garda security van";
char pass[] = "*SaMiScOoL*";

// Use Virtual pin 5 for uptime display
// #define PIN_UPTIME V5

// This function tells Arduino what to do if there is a Widget
// which is requesting data for Virtual Pin (5)
//BLYNK_READ(PIN_UPTIME)
//{
//  // This command writes Arduino's uptime in seconds to Virtual Pin (5)
//  Blynk.virtualWrite(PIN_UPTIME, millis() / 1000);
//}

BlynkTimer timer; // Announcing the timer
WidgetLCD lcd(V3);


void setup()
{
  // Debug console
  Serial.begin(9600);

  // init wash struct
  myWash.threshold = THRESHOLD;
  myWash.prevState = WASH_DONE; // set different to force an update
  myWash.curState = WASH_IDLE;
  myWash.lastActiveTime = millis();
  myWash.lastIdleTime = myWash.lastActiveTime;
  myWash.wasRecentlyActive = false;

  // Enable the MPU6050 - wake it up!
  Wire.begin();
  Wire.beginTransmission(MPU_addr);
  Wire.write(0x6B);  // PWR_MGMT_1 register
  Wire.write(0);     // set to zero (wakes up the MPU-6050)
  Wire.endTransmission(true);

  Wire.beginTransmission(MPU_addr);
  Wire.write(0x1B); // write to register starting at 0x1B
  Wire.write(0x00); // Self Tests Off and set Gyro FS to 250
  Wire.write(ACCEL_SCALE_2G); // Self Tests Off and set Accl FS to 2g
  Wire.endTransmission(true);

  for (int i = 0; i < 10; i++)
  {
    mpu6050_getAccel();
    accelX_init += accelX;
    accelY_init += accelY;
    accelZ_init += accelZ;
    delay(100);
  }
  accelX_init /= 10;
  accelY_init /= 10;
  accelZ_init /= 10;

  if (DEBUG)
  {
    Serial.print("accelX_init: "); Serial.print(accelX_init); Serial.print(" | "); Serial.println(accelX_init / MPU_ACCL_2_SCALE);
    Serial.print("accelY_init: "); Serial.print(accelY_init); Serial.print(" | "); Serial.println(accelY_init / MPU_ACCL_2_SCALE);
    Serial.print("accelZ_init: "); Serial.print(accelZ_init); Serial.print(" | "); Serial.println(accelZ_init / MPU_ACCL_2_SCALE);
  }

  Blynk.begin(auth, ssid, pass);

  // initialize timer
  timer.setInterval(timeoutValue, myTimerEvent);
}


// Main Loop
void loop()
{
  Blynk.run();
  timer.run();
}


// State machine
void myTimerEvent()
{
  static float accForce = 0;
  static float meanValue = 0;
  static uint8_t meanCount = 0;

  long int t_now = millis();

  // Read from mpu6050
  mpu6050_getAccel();
  mpu6050_getTemp();

  accForce = fabs(accelX - accelX_init) + fabs(accelY - accelY_init) + fabs(accelZ - accelZ_init);
  accForce *= 9.81f / MPU_ACCL_2_SCALE; // scale it to a value we can understand

  if(meanCount++ < 10)
  {
    meanValue += accForce;
  }
  else 
  {
    meanValue /= 10;
    meanCount = 1;
  }

  if(fabs(accForce - meanValue) > threshold)
  {
    myWash.lastActiveTime = t_now;
  }

/* Previous code
  if (accForce > myWash.threshold)
  {
    myWash.lastActiveTime = t_now;
  }
*/

  // Did we exceed our threshold at any time in the last three seconds?
  // "Recently" active means our force exceeded the threshold at least once
  // within the last three seconds.
  myWash.wasRecentlyActive = (bool) ((t_now - myWash.lastActiveTime) < TIME_WINDOW);

  if (DEBUG)
  {
    Serial.print("accForce: "); Serial.print(accForce); Serial.print(" | "); Serial.println(myWash.threshold);
    Serial.print("wasRecentlyActive: "); Serial.println(myWash.wasRecentlyActive);
    Serial.print("State: "); Serial.println(stateStrings[myWash.curState]);
  }

  if (myWash.prevState != myWash.curState)
  {
    myWash.prevState = myWash.curState;
    lcd.clear();
    lcd.print(0, 0, stateStrings[myWash.curState]);
  }

  switch (myWash.curState)
  {
    case (WASH_IDLE):
      if (myWash.wasRecentlyActive)
      {
        // Whenever there's so much as a twitch, we switch to the WASH_MAYBE_ON
        // state.
        setState(WASH_MAYBE_ON);
      }
      else
      {
        myWash.lastIdleTime = t_now;
      }
      break;
    case (WASH_MAYBE_ON):
      if (myWash.wasRecentlyActive)
      {
        // How long have we been in this state?
        if (t_now > (myWash.lastIdleTime + TIME_UNTIL_ON))
        {
          // For a while now! We must be in a cycle!
          setState(WASH_ON);
        }
        else
        {
          // Wait and see.
        }
      }
      else
      {
        // No vibration in the last three seconds. False alarm!
        setState(WASH_IDLE);
      }
      break;
    case (WASH_ON):
      if (myWash.wasRecentlyActive)
      {
        // This matches our expectation, so we must be in the right state.
      }
      else
      {
        // We stopped vibrating. We might be off.
        setState(WASH_MAYBE_DONE);
      }
      break;
    case (WASH_MAYBE_DONE):
      if (myWash.wasRecentlyActive)
      {
        // We thought we were done, but we’re vibrating again. False alarm!
        setState(WASH_ON);
      }
      else if (t_now > (myWash.lastActiveTime + TIME_UNTIL_DONE))
      {
        // We’ve been in this state for a while now. We must be done with a cycle.
        setState(WASH_DONE);
      }
      break;
    case (WASH_DONE):
      // Nothing to do except reset now.
      setState(WASH_IDLE);
      Blynk.notify("PICK UP YER SHITEY CLOTHES");
      break;
  }

  // send all the goodies
  Blynk.virtualWrite(PIN_UPTIME, t_now / 1000);
  Blynk.virtualWrite(PIN_ACCEL, accForce);
  Blynk.virtualWrite(PIN_TEMPERATURE, temperature / 340.00f + 36.53f);
}


void setState(washState newState)
{
  myWash.curState = newState;
  if (DEBUG)
  {
    Serial.print("state change: "); Serial.println(stateStrings[newState]);
  }
}

void mpu6050_getAccel()
{
  // Read from accel
  Wire.beginTransmission(MPU_addr);
  Wire.write(0x3B);  // starting with register 0x3B (ACCEL_XOUT_H)
  Wire.endTransmission(false);
  Wire.requestFrom(MPU_addr, 6, true); // request a total of 8 registers
  accelX = Wire.read() << 8 | Wire.read(); // 0x3B (ACCEL_XOUT_H) & 0x3C (ACCEL_XOUT_L)
  accelY = Wire.read() << 8 | Wire.read(); // 0x3D (ACCEL_YOUT_H) & 0x3E (ACCEL_YOUT_L)
  accelZ = Wire.read() << 8 | Wire.read(); // 0x3F (ACCEL_ZOUT_H) & 0x40 (ACCEL_ZOUT_L)
}

void mpu6050_getTemp()
{
  // Read from accel
  Wire.beginTransmission(MPU_addr);
  Wire.write(0x41);  // starting with register 0x3B (TEMP_OUT_H)
  Wire.endTransmission(false);
  Wire.requestFrom(MPU_addr, 2, true); // request a total of 8 registers
  temperature = Wire.read() << 8 | Wire.read(); // 0x41 (TEMP_OUT_H) & 0x42 (TEMP_OUT_L)
}
