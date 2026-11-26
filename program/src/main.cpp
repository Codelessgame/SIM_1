#include <Arduino.h>


#define PinResetButton 0;

#define EnA 20;
#define EnB 21;
#define In1 22;
#define In2 23;
#define In3 24;
#define In4 25;

enum CarState {
  StartBlock,
  FollowLine,
  PauseBlock,
  EndBlock,
};

void setup() {
  Serial.begin(115200);
}

void loop() {
  Serial.write("hello");

  delay(2000);
}

// put function definitions here:
int myFunction() {
}