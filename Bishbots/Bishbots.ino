#include <SoftwareServo.h>

SoftwareServo myservo;  // create servo object to control a servo
// a maximum of eight servo objects can be created

uint8_t pin_IR = A1;
unsigned int irValue = 0;

uint8_t pinBuzzer = 1;
uint8_t pinServo = 0;

int buzzerValue = 50; // value from 0-255 for tone
unsigned long delayShort = 300;
unsigned long delayLong = 500;

int servoPos = 0; // position in degrees

// IR LED threshold for a bot nearby
unsigned int botNearby = 90;
unsigned int armSwings = 0;

void setup() {
  // Initialize pins
  pinMode(pin_IR, INPUT);
  pinMode(pinBuzzer, OUTPUT);

  // initialize the servo
  myservo.attach(pinServo);  // attaches the servo on pin 9 to the servo object
  myservo.write(0);
  SoftwareServo::refresh();
  delay(100);

  // Light/buzzer race countdown
  for (int i = 0; i < 3; i++) {
    analogWrite(pinBuzzer, buzzerValue);
    delay(delayLong);
    analogWrite(pinBuzzer, LOW);
    delay(delayShort);
  }

  analogWrite(pinBuzzer, 100);
  delay(1300);
  analogWrite(pinBuzzer, LOW);

  //  Serial.begin(9600);
}

void loop() {
  // put your main code here, to run repeatedly:
  irValue = analogRead(A1);

  if (irValue > botNearby) {
    while (armSwings++ < 3) {
      for (servoPos = 0; servoPos <= 180; servoPos += 10) { // goes from 0 degrees to 180 degrees
        // in steps of 1 degree
        myservo.write(servoPos);              // tell servo to go to position in variable 'pos'
        delay(15);                       // waits 15ms for the servo to reach the position
        SoftwareServo::refresh();
      }
      for (servoPos = 180; servoPos >= 0; servoPos -= 10) { // goes from 180 degrees to 0 degrees
        myservo.write(servoPos);              // tell servo to go to position in variable 'pos'
        delay(15);                       // waits 15ms for the servo to reach the position
        SoftwareServo::refresh();
      }
    }
    armSwings = 0;
    //  }



    //  Serial.println(irValue);
    //  delay(1000);

    delay(500);
  }
}

