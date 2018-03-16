const int ledPin = 9;

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  pinMode(ledPin, OUTPUT);
}

void loop() {
  byte brightness;
  // put your main code here, to run repeatedly:

  if(Serial.available()){

    brightness = Serial.read();

    analogWrite(ledPin, brightness);
  }
}
