int potPin = A1;
int outputPin = 4;

void setup() {
  // put your setup code here, to run once:
  pinMode(outputPin, OUTPUT);
  pinMode(13, OUTPUT);
  pinMode(potPin, INPUT);
  digitalWrite(outputPin, LOW);
}

void loop() {
  // put your main code here, to run repeatedly:
  if (PORTD & B00010000) {
    PORTD = B00000000;
    digitalWrite(13, LOW);
  } else {
    PORTD = B00010000;
    digitalWrite(13, HIGH);
  }

//  delayMicroseconds(map(analogRead(potPin), 0 , 1023, 1, 10000));
  delay(map(analogRead(potPin), 0 , 1023, 1, 2000));
}

