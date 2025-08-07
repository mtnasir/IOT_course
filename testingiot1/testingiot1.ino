int counter=0;
void setup() {
  // put your setup code here, to run once:
pinMode(2,OUTPUT);
pinMode(32,OUTPUT);
Serial.begin(9600);
}

void loop() {
  // put your main code here, to run repeatedly:
// digitalWrite(2,HIGH);
analogWrite(2,50);
delay(2000);
analogWrite(2,150);
delay(2000);
analogWrite(2,255);
delay(2000);
// // digitalWrite(2,LOW);
// delay(2000);
// int data=analogRead(32);
Serial.println(counter);
counter++;
}
