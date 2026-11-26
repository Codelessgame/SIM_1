#include <Arduino.h>


#define PinResetButton 0 

// MOTOR CONTROLLERS
#define EnA 20
#define EnB 21
#define In1 22
#define In2 23
#define In3 24
#define In4 25

struct Motor{
  int EnginePin;
  int Input1;
  int Input2;
};


#define FORWARD 1
#define BACKWARD -1

Motor Motor1 = {EnA, In1, In2};
Motor Motor2 = {EnB, In3, In4};

#define turnspeed 50
#define forwardspeed 100

// IFRARED SENSORS 
#define IR1_PIN 30
#define IR1_PIN 31
#define IR3_PIN 32

// infrared sensors ArrayLookup likely to change so I will use it as variables
#define left 1
#define middle 0 
#define right 2

int IRCurrentSensorsValues[3] = {0,0,0};
int IRPastSensorsValues[3] = {0,0,0};
int IRSensorDifferenceValues[3] = {0,0,0};
int IRSensorsPins[3] = {IR1_PIN, IR1_PIN, IR3_PIN};

// used in checking which state is correct
int timer =0;

enum States {
  StartBlock,
  FollowLine,
  PauseBlock,
  CheckBlock,
  EndBlock,
  Manual,
};
States CarState=Manual; 

void _PauseBlock();
void _FollowLine();

// MANUAL CONTROL & BLUETOOTH (Arduino Mega 2560 Serial1: RX1=19, TX1=18)
#define BT_SERIAL Serial1
#define BT_BAUD 9600
int manualSpeed = forwardspeed;

void _ManualControl();
void MoveBackward();
void StopMotors();
void ProcessManualCommand(char cmd, Stream& port);
void PrintManualHelp(Stream& port);

void setup() {
  Serial.begin(115200);
  BT_SERIAL.begin(BT_BAUD);

  pinMode(EnA, OUTPUT);
  pinMode(EnB, OUTPUT);
  pinMode(In1, OUTPUT);
  pinMode(In2, OUTPUT);
  pinMode(In3, OUTPUT);
  pinMode(In4, OUTPUT);
  StopMotors();

  PrintManualHelp(Serial);
  PrintManualHelp(BT_SERIAL);
}

void loop() {
  switch (CarState)
  {
  case StartBlock:
    break;
  case FollowLine:
    _FollowLine();
    break;
  case PauseBlock:
   _PauseBlock();
    break;
  case EndBlock:
    break;
  case Manual:
    _ManualControl();
    break;
  }

  
}

void SetMotorRaw(Motor motor, int direction, int motorspeed=100){
  if (direction == FORWARD) {
      digitalWrite(motor.Input1 ,HIGH);
      digitalWrite(motor.Input2,LOW);
      analogWrite(motor.EnginePin, motorspeed);
    } else {
      digitalWrite(motor.Input1, LOW);
      digitalWrite(motor.Input2, HIGH);
      analogWrite(motor.EnginePin, motorspeed);
    }; 
};

void StopMotor(Motor motor){
  digitalWrite(motor.Input1, LOW);
  digitalWrite(motor.Input2, LOW);
}

void StopMotors(){
  StopMotor(Motor1);
  StopMotor(Motor2);
}

void MoveForward(){
  SetMotorRaw(Motor1, FORWARD, forwardspeed);
  SetMotorRaw(Motor2, FORWARD, forwardspeed);
}

void MoveBackward(){
  SetMotorRaw(Motor1, BACKWARD, manualSpeed);
  SetMotorRaw(Motor2, BACKWARD, manualSpeed);
}

void TurnLeft(){
  SetMotorRaw(Motor1, FORWARD, turnspeed);
  SetMotorRaw(Motor2, BACKWARD, turnspeed);
}

void TurnRight(){
  SetMotorRaw(Motor1, FORWARD, turnspeed);
  SetMotorRaw(Motor2, BACKWARD, turnspeed);
}


void ReadIRSensor(){
  for(int i=0; i<3;i++){
    IRPastSensorsValues[i]= IRCurrentSensorsValues[i];
    IRCurrentSensorsValues[i] = digitalRead(IRSensorsPins[i]);
    IRSensorDifferenceValues[i] = IRCurrentSensorsValues[i] - IRPastSensorsValues[i];
  }

}
void _FollowLine(){
  ReadIRSensor();

  if(IRCurrentSensorsValues[left]!=0 && IRCurrentSensorsValues[middle] !=0 && IRCurrentSensorsValues[right] !=0){
    StopMotors();
    CarState=CheckBlock;
    timer = 0;
    return;
  }
  // here Check if it needs to turn or change state or 
  if (IRCurrentSensorsValues[left]==0 && IRCurrentSensorsValues[right]==0){
    MoveForward();
  }
  else if (IRCurrentSensorsValues[left]!=0 && IRCurrentSensorsValues[right]==0){
    // turn right
    TurnRight();
  }
  else if (IRCurrentSensorsValues[left]==0 && IRCurrentSensorsValues[right]!=0){
    // turn left
    TurnLeft();
  }


}

void _CheckBlock(){
  MoveForward();
  delay(100);
  StopMotors();
  ReadIRSensor();
  if(IRCurrentSensorsValues[left]!=0 && IRCurrentSensorsValues[middle] !=0 && IRCurrentSensorsValues[right] !=0){
    CarState=PauseBlock;
    return;
  }
  MoveForward();
  delay(100);
  StopMotors();
  ReadIRSensor();
  if(IRCurrentSensorsValues[left]!=0 && IRCurrentSensorsValues[middle] !=0 && IRCurrentSensorsValues[right] !=0){
    CarState=EndBlock;
    return;
  } 



}

void _PauseBlock(){
  
}


// BELOW ARE ONLY MANUAL COMMAND FUNCTIONS
void PrintManualHelp(Stream& port){
  port.println(F("=== Manual Control Ready ==="));
  port.println(F("W/F: Forward | S/B: Backward | A/L: Left | D/R: Right"));
  port.println(F("X/Space/0: Stop | +/-: Speed +/-25 | 1-9: Speed preset | H/?: Help"));
}

void ProcessManualCommand(char cmd, Stream& port){
  if (cmd == '\r' || cmd == '\n') {
    return;
  }

  switch (cmd) {
    case 'w':
    case 'W':
    case 'f':
    case 'F':
      SetMotorRaw(Motor1, FORWARD, manualSpeed);
      SetMotorRaw(Motor2, FORWARD, manualSpeed);
      port.print(F("Forward @ "));
      port.println(manualSpeed);
      break;

    case 's':
    case 'S':
    case 'b':
    case 'B':
      MoveBackward();
      port.print(F("Backward @ "));
      port.println(manualSpeed);
      break;

    case 'a':
    case 'A':
    case 'l':
    case 'L':
      SetMotorRaw(Motor1, FORWARD, manualSpeed);
      SetMotorRaw(Motor2, BACKWARD, manualSpeed);
      port.print(F("Turn Left @ "));
      port.println(manualSpeed);
      break;

    case 'd':
    case 'D':
    case 'r':
    case 'R':
      SetMotorRaw(Motor1, BACKWARD, manualSpeed);
      SetMotorRaw(Motor2, FORWARD, manualSpeed);
      port.print(F("Turn Right @ "));
      port.println(manualSpeed);
      break;

    case 'x':
    case 'X':
    case ' ':
    case '0':
      StopMotors();
      port.println(F("Stopped"));
      break;

    case '+':
    case '=':
      manualSpeed = min(255, manualSpeed + 25);
      port.print(F("Speed: "));
      port.println(manualSpeed);
      break;

    case '-':
    case '_':
      manualSpeed = max(0, manualSpeed - 25);
      port.print(F("Speed: "));
      port.println(manualSpeed);
      break;

    case '1' ... '9':
      manualSpeed = map(cmd - '0', 1, 9, 50, 255);
      port.print(F("Speed: "));
      port.println(manualSpeed);
      break;

    case 'h':
    case 'H':
    case '?':
      PrintManualHelp(port);
      break;

    default:
      port.print(F("Unknown cmd: "));
      port.println(cmd);
      break;
  }
}

void _ManualControl(){
  while (Serial.available() > 0) {
    char cmd = static_cast<char>(Serial.read());
    ProcessManualCommand(cmd, Serial);
  }
  while (BT_SERIAL.available() > 0) {
    char cmd = static_cast<char>(BT_SERIAL.read());
    ProcessManualCommand(cmd, BT_SERIAL);
  }
}


