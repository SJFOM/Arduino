#include <IRremote.h>
const int PIN_SEND = 12;
const int PIN_RECEIVE = 11;

IRrecv irrecv(PIN_RECEIVE);

decode_results results;

void setup()
{
  Serial.begin(9600);
  irrecv.enableIRIn(); // Start the receiver
  pinMode(PIN_SEND, OUTPUT);
}

void loop() {
  if (irrecv.decode(&results)) {
    Serial.println(results.value, HEX);
    irrecv.resume(); // Receive the next value
  }
  digitalWrite(PIN_SEND, HIGH);
  delay(1000);
  digitalWrite(PIN_SEND, LOW);
  delay(1000);
}
