int potPin = A1;
int outputPin = 4;

void setup() {
  // put your setup code here, to run once:
  pinMode(outputPin, OUTPUT);
  pinMode(13, OUTPUT);
  pinMode(potPin, INPUT);
  digitalWrite(outputPin, LOW);
  blipper(200, 1000, 300);
  blipper(200, 1000, 300);
  blipper(200, 1000, 300);

  blipper(300, 1250, 0);
  blipper(200, 840, 0);
  blipper(200, 1000, 300);
  blipper(300, 1250, 0);
  blipper(200, 840, 0);
  blipper(400, 1000, 300);

  blipper(200, 720, 300);
  blipper(200, 720, 300);
  blipper(200, 720, 300);
  blipper(200, 670, 200);
  blipper(200, 840, 0);
  blipper(200, 1000, 300);
  blipper(300, 1250, 0);
  blipper(200, 840, 0);
  blipper(200, 1000, 300);
}

void loop() {
  // put your main code here, to run repeatedly:
  //  if (PORTD & B00010000) {
  //    PORTD = B00000000;
  //    digitalWrite(13, LOW);
  //  } else {
  //    PORTD = B00010000;
  //    digitalWrite(13, HIGH);
  //  }


  //  for (int i = 1; i < 10000; i++) {
  //    PORTD ^= B00010000;
  //    delayMicroseconds(1000);
  //  }

  //  delayMicroseconds(map(analogRead(potPin), 0 , 1023, 1, 100000));
  //  delay(map(analogRead(potPin), 0 , 1023, 1, 2000));
}

void blipper(int time, int tone, int wait) {
  for (int i = 1; i < time; i++) {
    PORTD ^= B00010000;
    delayMicroseconds(tone);
  }
  delay(wait);
}

