int x = 0;

void setup() 
{
  // put your setup code here, to run once:
  // Enable Serial Communication. Set Baud Rate to 115200
  Serial.begin(115200);
}

void loop() 
{
  // put your main code here, to run repeatedly:
  // Send New Integer to ROS 2 Node
  Serial.println(x);
  x++;
  delay(1000);

}
