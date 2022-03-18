#include "Servo.h"
#include "LedControl.h"
#include "SoftwareSerial.h"
#include "olbin.h"

SoftwareSerial mySerial(4,5);
Servo microServo;
LedControl dot = LedControl(11,13,12,1);
 
int ultraSonic_trig = 7;
int ultraSonic_echo = 6;
int button = 2;
int servo = 10;
int distance =0;
bool isUltraSonicCheck = false;
bool button_clicked = false;
int32_t robot_speed;
int speed_factor;
Olbin olbin;
InfoToMakeDXLPacket_t _CM50_command;

int32_t LED_ON = 1;
int32_t LED_OFF = 0;
/* Control protocol
@[Target(1)][Length(2)][Data]#
- Target:
  M : Motor
  L : Led
  U : Ultra Sonic
  S : Servo
- Data:
    Motor :
      MF : Move forward
      MB : Move backward
      TR : Turn Right
      TL : Turn Left
      ST : Stop
      SL : Set speed to slow (200)
      SM : Set speed to medium (256)
      SH : Set speed to fast (300)
    LED :
      BN : Blue LED On
      BF : Blue LED Off
      RN : Red LED On
      RF : Red LED Off
    Ultra Sonic :
      Data None.
      Return [distance in cm]
    Servo :
      Angle (Example @S290# : Set servo angle to 90 dgree)
      
*/

void setup() {
  mySerial.begin(57600);
  pinMode(ultraSonic_trig, OUTPUT);
  pinMode(ultraSonic_echo, INPUT);
  pinMode(button, INPUT);
  microServo.attach(servo);
  dot.shutdown(0, false);
  dot.setIntensity(0,8);
  dot.clearDisplay(0);
  
  attachInterrupt(digitalPinToInterrupt(button), catchInterrupt, RISING);
  Serial.begin(115200);
  while (!Serial) {
    ; // wait for serial port to connect. Needed for Native USB only
  }

  olbin = Olbin();
  speed_factor = 50;
  change_speed(250);
  
  
  Serial.println("Start");

}

void loop(){
  loop_uploadMode();
  //loop_testMode();
 
}

void loop_testMode() {
  // Unblock loop_testMode in loop() and put test code here.
}

void loop_uploadMode() {
  if(button_clicked)
  {
    change_speed_by_button();
    button_clicked = false;
  }
  // CM-50 Control code
  String recv_data=Serial.readStringUntil('#');
  if(recv_data.charAt(0) == '@')
  {
    char target = recv_data.charAt(1);
    int data_len = recv_data.substring(2,4).toInt();
    //int data_len = recv_data.charAt(2) - '0';
    String data = recv_data.substring(4, 4 + data_len);

    switch(target){
      case 'M': // Motor control
        if(data.equals("MF")) {
          moveForward();
        }
        else if(data.equals("MB")){
          moveBackward();
        }
        else if(data.equals("ST")){
          stopMoving();
        }
        else if(data.equals("TR")) {
          turnRight();
        }
        else if(data.equals("TL")) {
          turnLeft();
        }
        else if(data.equals("SL")) {
          change_speed(200);
        }
        else if(data.equals("SM")) {
          change_speed(250);
        }
        else if(data.equals("SH")) {
          change_speed(300);
        }
        break;

      case 'L':
        if(data.equals("BN"))
        {
          turnBlueLEDOn();
        }
        else if(data.equals("BF"))
        {
          turnBlueLEDOff();
        }
        else if(data.equals("RN"))
        {
          turnRedLEDOn();
        }
        else if(data.equals("RF"))
        {
          turnRedLEDOff();
        }
        break;
      case 'U': // Ultrasonic sensor control
        distance = getDistance();
        Serial.println(distance);
        break;
      case 'S': // Servo control
        microServo.write(data.toInt());
        break;
      case 'D': //Dot Matrix
        displayAtDotMatrix(data);
        break;
    }
  }

}

void catchInterrupt()
{
  button_clicked = true;
}

///////////////////////////////////////////////////////////////////////////
//
// Control methods
//
///////////////////////////////////////////////////////////////////////////


/// Ultrasonic sensor
int getDistance()
{
  if(isUltraSonicCheck)
  {
    return distance;
  }
  isUltraSonicCheck = true;
  digitalWrite(ultraSonic_trig, HIGH);
  delayMicroseconds(10);
  digitalWrite(ultraSonic_trig, LOW);
  distance = pulseIn(ultraSonic_echo,HIGH) * 340 /2 / 10000;
  isUltraSonicCheck = false;
  return distance;
}

/// End of Ultrasonic sensor

/// Dot-Matrix Display
void displayAtDotMatrix(String data)
{
  int index = 63;
  for(int col = 0; col <8; col++)
  {
    for(int row = 7; row >= 0; row--)
    {
      int ledState = data.charAt(index) - '0';
      dot.setLed(0,col,row,ledState);
      index--;
    }
  }
}
/// End of Dot-Matrix Display

///////////////////////////////////////////////////////////////////////////
//
// CM-50 Control
//
///////////////////////////////////////////////////////////////////////////

void change_speed_by_button()
{
  
  speed_factor = (speed_factor+50)%150; // Rotate 0 - 50 - 100
  change_speed(200 + speed_factor); // Speed rotate 200 - 250 - 300
  switch(robot_speed)
  {
    case 200:
      turnRedLEDOn();
      turnBlueLEDOff();
      break;
    case 250:
      turnRedLEDOff();
      turnBlueLEDOff();
      break;
    case 300:
      turnRedLEDOff();
      turnBlueLEDOn();
      break;
  }
  stopMoving();

}


void change_speed(int32_t speed_to_change)
{
  robot_speed = speed_to_change;
}

void write_command(InfoToMakeDXLPacket_t command)
{
  mySerial.write(command.p_packet_buf, command.generated_packet_length);
  delay(100);
}


/// CM-50 Motor

void moveForward()
{
  _CM50_command = olbin.command_set_motor_speed(1, MOTOR_DIRECTION::CCW, robot_speed);
  write_command(_CM50_command);
  _CM50_command = olbin.command_set_motor_speed(2, MOTOR_DIRECTION::CW, robot_speed);
  write_command(_CM50_command);
}

void moveBackward()
{
  _CM50_command = olbin.command_set_motor_speed(1, MOTOR_DIRECTION::CW, robot_speed);
  write_command(_CM50_command);
  _CM50_command = olbin.command_set_motor_speed(2, MOTOR_DIRECTION::CCW, robot_speed);
  write_command(_CM50_command);
}

void turnRight()
{
  _CM50_command = olbin.command_set_motor_speed(1, MOTOR_DIRECTION::CW, robot_speed);
  write_command(_CM50_command);
  _CM50_command = olbin.command_set_motor_speed(2, MOTOR_DIRECTION::CW, robot_speed);
  write_command(_CM50_command);
}

void turnLeft()
{
  _CM50_command = olbin.command_set_motor_speed(1, MOTOR_DIRECTION::CCW, robot_speed);
  write_command(_CM50_command);
  _CM50_command = olbin.command_set_motor_speed(2, MOTOR_DIRECTION::CCW, robot_speed);
  write_command(_CM50_command);
}


void stopMoving()
{
  _CM50_command = olbin.command_set_motor_speed(1, MOTOR_DIRECTION::CW, 0);
  write_command(_CM50_command);
  _CM50_command = olbin.command_set_motor_speed(2, MOTOR_DIRECTION::CW, 0);
  write_command(_CM50_command);
  delay(1000);
}

/// End of CM-50 Motor

// CM-50 LED
void turnBlueLEDOn()
{
  _CM50_command = olbin.get_command(200, 80, (uint8_t*)&LED_ON, 1);
  write_command(_CM50_command);
}

void turnBlueLEDOff()
{
  _CM50_command = olbin.get_command(200, 80, (uint8_t*)&LED_OFF, 1);
  write_command(_CM50_command);
}

void turnRedLEDOn()
{
  _CM50_command = olbin.get_command(200, 79, (uint8_t*)&LED_ON, 1);
  write_command(_CM50_command);
}

void turnRedLEDOff()
{
  _CM50_command = olbin.get_command(200, 79, (uint8_t*)&LED_OFF, 1);
  write_command(_CM50_command);
}

// End of CM-50 LED
