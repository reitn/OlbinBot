#include "SoftwareSerial.h"
#include "olbin.h"

SoftwareSerial mySerial(2,3);
int ultraSonic_trig = 7;
int ultraSonic_echo = 6;
int distance =0;
bool isUltraSonicCheck = false;
/* Control protocol
@[Target][Length][Data]#
- Target:
  M : Motor
  L : Led
  U : Ultra Sonic
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
      Return @[distance in cm]#
*/

// CM-50 Commands (Control bytes)
int32_t robot_speed;
Olbin olbin;
InfoToMakeDXLPacket_t _CM50_command;


void setup() {
  mySerial.begin(57600);
  pinMode(ultraSonic_trig, OUTPUT);
  pinMode(ultraSonic_echo, INPUT);
  Serial.begin(115200);
  while (!Serial) {
    ; // wait for serial port to connect. Needed for Native USB only
  }

  // Generate CM-50 Commands
  olbin = Olbin();
  change_speed(256);
  Serial.println("Start");

}

void loop() {

  // CM-50 Control code
  String recv_data=Serial.readStringUntil('#');
  if(recv_data.charAt(0) == '@')
  {
    char target = recv_data.charAt(1);
    int data_len = recv_data.charAt(2) - '0';
    String data = recv_data.substring(3, 3 + data_len);

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
          change_speed(256);
        }
        else if(data.equals("SH")) {
          change_speed(300);
        }
        
        break;

      case 'L':
        break;
      case 'U': // Ultrasonic sensor control
        int distance = getDistance();
        String startMark = "@";
        String endMark = "#";
        String value = startMark + distance + endMark;
        //Serial.print("@");
        Serial.println(distance);
        //Serial.flush();
        //Serial.println("#");
        break;
    }
  }

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


///////////////////////////////////////////////////////////////////////////
//
// CM-50 Control
//
///////////////////////////////////////////////////////////////////////////


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

}

/// End of CM-50 Motor
