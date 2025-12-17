#define LED_PIN 13

void setup() 
{
  // Constructor Function. Use for settig up initial values
  // Configure Pin 13 (LED_PIN) to accept commands
  pinMode(LED_PIN, OUTPUT);

  // Turn off LED at startup
  digitalWrite(LED_PIN, LOW); 

  // Set Baud Rate
  Serial.begin(115200);

  // Wait for 1s to initialize 
  Serial.setTimeout(1);
}

void loop() 
{
  if (Serial.available())
  {
    int x = Serial.readString().toInt();
    if(x == 0)
    {
      // turn off LED
      digitalWrite(LED_PIN, LOW); 
    }
    else
    {
      // turn on the LED
      digitalWrite(LED_PIN, HIGH); 
    }
  }
  delay(0.1);
}
