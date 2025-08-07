
void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  pinMode(2, OUTPUT);
}

void loop() {
  // put your main code here, to run repeatedly:
  if (Serial.available() > 1) {
    int Rnumber = Serial.parseInt();
    while (Serial.available() > 1) {
      Serial.read();
    }
    //  analogWrite(2,Rnumber);
    dacWrite(26, Rnumber);
    Serial.print("DAC is = ");
    Serial.println(Rnumber);
    int Sensor = analogRead(36);
    Serial.print("Sensor reading  is = ");
    Serial.println(Sensor);
  }
}
