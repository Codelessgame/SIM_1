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

void setup() {
  Serial.begin(115200);
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


