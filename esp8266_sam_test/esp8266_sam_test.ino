/*************************************************************
  Download latest Blynk library here:
    https://github.com/blynkkk/blynk-library/releases/latest

  Blynk is a platform with iOS and Android apps to control
  Arduino, Raspberry Pi and the likes over the Internet.
  You can easily build graphic interfaces for all your
  projects by simply dragging and dropping widgets.

    Downloads, docs, tutorials: http://www.blynk.cc
    Sketch generator:           http://examples.blynk.cc
    Blynk community:            http://community.blynk.cc
    Follow us:                  http://www.fb.com/blynkapp
                                http://twitter.com/blynk_app

  Blynk library is licensed under MIT license
  This example code is in public domain.

 *************************************************************

  This example shows how to send values to the Blynk App,
  when there is a widget, attached to the Virtual Pin and it
  is set to some frequency

  Project setup in the app:
    Value Display widget attached to V5. Set any reading
    frequency (i.e. 1 second)
 *************************************************************/

/* Comment this out to disable prints and save space */
#define BLYNK_PRINT Serial


#include <ESP8266WiFi.h>
#include <BlynkSimpleEsp8266.h>

#include<Wire.h>

#define DEBUG 0

const int MPU_addr = 0x68; // I2C address of the MPU-6050
int16_t AcX, AcY, AcZ, Tmp;
const float MPU_ACCL_2_SCALE = 16384.0;
const float MPU_ACCL_4_SCALE = 8192.0;
const float MPU_ACCL_8_SCALE = 4096.0f;
const float MPU_ACCL_16_SCALE = 2048.0;

const uint8_t ACCEL_SCALE_2G = 0x00;
const uint8_t ACCEL_SCALE_4G = 0x01;
const uint8_t ACCEL_SCALE_8G = 0x10;
const uint8_t ACCEL_SCALE_16G = 0x11;

volatile float avgAccel = 0.0f;

// You should get Auth Token in the Blynk App.
// Go to the Project Settings (nut icon).
char auth[] = "67f7dc4a21a64f1e81a4a7087572cf2f";

// Your WiFi credentials.
// Set password to "" for open networks.
char ssid[] = "Garda security van";
char pass[] = "*SaMiScOoL*";

// Use Virtual pin 5 for uptime display
#define PIN_UPTIME V5

BlynkTimer timer; // Announcing the timer

// This function tells Arduino what to do if there is a Widget
// which is requesting data for Virtual Pin (5)
//BLYNK_READ(PIN_UPTIME)
//{
//  // This command writes Arduino's uptime in seconds to Virtual Pin (5)
//  Blynk.virtualWrite(PIN_UPTIME, millis() / 1000);
//}

void myTimerEvent()
{
  static uint8_t count = 0;
  static float accAvg = 0;
  Blynk.virtualWrite(PIN_UPTIME, millis() / 1000);
  if (count++ > 9)
  {
    count = 0;
    //    Blynk.notify("10 seconds bitch"); // send msg to phone
  }

  // Read from accel
  Wire.beginTransmission(MPU_addr);
  Wire.write(0x3B);  // starting with register 0x3B (ACCEL_XOUT_H)
  Wire.endTransmission(false);
  Wire.requestFrom(MPU_addr, 8, true); // request a total of 8 registers
  AcX = Wire.read() << 8 | Wire.read(); // 0x3B (ACCEL_XOUT_H) & 0x3C (ACCEL_XOUT_L)
  AcY = Wire.read() << 8 | Wire.read(); // 0x3D (ACCEL_YOUT_H) & 0x3E (ACCEL_YOUT_L)
  AcZ = Wire.read() << 8 | Wire.read(); // 0x3F (ACCEL_ZOUT_H) & 0x40 (ACCEL_ZOUT_L)
  Tmp = Wire.read() << 8 | Wire.read(); // 0x41 (TEMP_OUT_H) & 0x42 (TEMP_OUT_L)

  AcX *= 9.81f/MPU_ACCL_2_SCALE;
  AcY *= 9.81f/MPU_ACCL_2_SCALE;
  AcZ *= 9.81f/MPU_ACCL_2_SCALE;

  accAvg = sqrt(AcX*AcX + AcY*AcY + AcZ*AcZ);
  avgAccel += accAvg;

//  Serial.print("avgAccel: "); Serial.println(accAvg*9.81f/MPU_ACCL_2_SCALE);
//  Serial.print("avgAccel: "); Serial.println(accAvg);

  if(++count >= 10)
  { 
//    Blynk.virtualWrite(V0, avgAccel*0.981f/MPU_ACCL_2_SCALE);
    Blynk.virtualWrite(V0, avgAccel/10.0f);
    Blynk.virtualWrite(V4, Tmp / 340.00f + 36.53f);
    count = 0;
    avgAccel = 0;
  }

  if (DEBUG)
  {
    Serial.print("AcX = "); Serial.print(9.81f * AcX / MPU_ACCL_2_SCALE);
    Serial.print(" | AcY = "); Serial.print(9.81f * AcY / MPU_ACCL_2_SCALE);
    Serial.print(" | AcZ = "); Serial.print(9.81f * AcZ / MPU_ACCL_2_SCALE);
    Serial.print(" | Tmp = "); Serial.println(Tmp / 340.00 + 36.53); //equation for temperature in degrees C from datasheet
  }
}

void setup()
{
  Wire.begin();

  // Enable the MPU6050 - wake it up!
  Wire.beginTransmission(MPU_addr);
  Wire.write(0x6B);  // PWR_MGMT_1 register
  Wire.write(0);     // set to zero (wakes up the MPU-6050)
  Wire.endTransmission(true);

  Wire.beginTransmission(MPU_addr);
  Wire.write(0x1B); // write to register starting at 0x1B
  Wire.write(0x00); // Self Tests Off and set Gyro FS to 250
  Wire.write(ACCEL_SCALE_2G); // Self Tests Off and set Accl FS to 2g
  Wire.endTransmission(true);

  // Debug console
  Serial.begin(9600);

  Blynk.begin(auth, ssid, pass);
  // You can also specify server:
  //Blynk.begin(auth, ssid, pass, "blynk-cloud.com", 80);
  //Blynk.begin(auth, ssid, pass, IPAddress(192,168,1,100), 8080);
  timer.setInterval(100L, myTimerEvent);
}

void loop()
{
  Blynk.run();
  timer.run();
}
