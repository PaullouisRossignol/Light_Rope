
// initialize output pin 
int output = 13;
int yaou = 0;
// the setup routine runs once when you press reset:
void setup() {
  // initialize serial communication at 9600 bits per second:
  Serial.begin(115200);
}


// the loop routine runs over and over again forever:
void loop() {
  
  // initialize the digital pin as an output.
  pinMode(output, OUTPUT);
  // read the input on analog pin 1:
  int sensorValue = analogRead(A1);
  // Convert the analog reading (which goes from 0 - 1023) to a voltage (0 - 5V):
  float voltage = sensorValue * (5.0 / 1023.0);
  String state = "";
  if(voltage > 0.5)
  {
      Serial.println("HIGH ");
      //Serial.println(voltage);
      digitalWrite(output, HIGH);   // Output one if detecting light
      yaou = 0;
  }
  else
  {
    if(yaou < 5)
    {
      yaou ++;
      Serial.println("LOW ");
    }
    //Serial.println(voltage);
    digitalWrite(output, LOW);   // Output zero if detecting light

  }
}
