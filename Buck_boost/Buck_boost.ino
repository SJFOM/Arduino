const int potPin = A1;
const int pwmPin = 5;

int sensorValue = 0;
int outputValue = 0;


void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  pinMode(potPin, INPUT);
  pinMode(pwmPin, OUTPUT);
  digitalWrite(pwmPin, LOW);
}

void loop() {
  // put your main code here, to run repeatedly:
  sensorValue = analogRead(potPin);

  outputValue = map(sensorValue, 0, 1023, 0, 255);

  analogWrite(pwmPin, outputValue);

  Serial.print("pot: ");
  Serial.print(sensorValue);
  Serial.print("\t");
  Serial.print("pwm: ");
  Serial.println(outputValue);
  delay(2);
}
