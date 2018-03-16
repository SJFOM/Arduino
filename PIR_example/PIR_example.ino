#define PIN_PIR_INPUT    (8)
#define PIN_LED_OUTPUT   (13)

void setup() {
  // put your setup code here, to run once:
  pinMode(PIN_PIR_INPUT, INPUT);
}

void loop() {
  // put your main code here, to run repeatedly:
  if (digitalRead(PIN_PIR_INPUT) == HIGH) {
    digitalWrite(PIN_LED_OUTPUT, HIGH);
  } else {
    digitalWrite(PIN_LED_OUTPUT, LOW);
  }
}
