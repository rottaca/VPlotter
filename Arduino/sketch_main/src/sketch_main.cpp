#include <Arduino.h>


// Uncomment to enable debug prints
//#define DEBUG_PRINTS

#include "hardwareCtrl.h"

// Size of the ringbuffer that contains all commands
#define CMD_RING_BUFFER_SIZE 128
// Size of the data that is read as a single piece
#define MAX_CMD_SIZE 64
// Maximum number of gcode parameters
#define MAX_GCODE_PARAMS 5
// Seperator for gcode parameters
#define GCODE_PARAM_SEPERATOR " "
// Seperator for gcode parameters
#define GCODE_COMMAND_SEPERATOR '\n'
// Timeout for serial read
#define SERIAL_TIMEOUT 100


#define SEND_NOERROR Serial.println("ACK: 0")
#define SEND_ERROR(err) Serial.println("ACK: " + String(err))
enum ERROR_CODES{
  ERROR_CALIB_FIRST = 1,
  ERROR_INVALID_PARAM,
  ERROR_UNKNOWN_CODE,
  ERROR_CALIB_FAILED
};
#define SEND_BUSY Serial.println("BUSY");

// Forward declarations
void processSerialInput();
void printCmd(char* cmd, char** params);
void executeCmd(char* cmd, char** params);
bool executeGCode(int code, char** params) ;
void executeMCode(int code, char** params) ;
size_t recieveSerialInput(char* data);
void processSerialInput(char* data);
void executeM7();
void executeM8();

boolean executeG0(char** params);
boolean executeG28(char** params);
boolean executeM6(char** params);
boolean executeM5(char** params);

// Variables for serial communication
//fifo_t buffer;

// Systemstate
boolean absolutePositioning = true;

void setup() {

  // Start serial port
  Serial.begin(9600);
  Serial.setTimeout(SERIAL_TIMEOUT);

  // Initialize ringbuffer
  //buffer = fifo_init(CMD_RING_BUFFER_SIZE + 1);
  hw_ctrl_init();
  Serial.println("System initialized");
  Serial.println("Version 0.1");

}

void loop() {
  // Always get bytes from buffer to avoid buffer overflows on serial port
  char input[MAX_CMD_SIZE+1];
  if(recieveSerialInput(input)<= 0)
    return;

  while(hw_ctrl_is_moving()){
      delay(500);
      SEND_BUSY;
  }

  processSerialInput(input);
}


size_t recieveSerialInput(char* data) {

    size_t sz =  Serial.readBytesUntil(GCODE_COMMAND_SEPERATOR, data, MAX_CMD_SIZE);
    data[sz] = 0;    // Replace seperator with string termination
    return sz;
}

/**
 * Processes incoming G-Code commands
 */
void processSerialInput(char* input) {

  // Split data into cmd and parameters
  char* params[MAX_GCODE_PARAMS];
  // First token is command
  char* cmd = strtok(input, GCODE_PARAM_SEPERATOR);

  // extract params
  char* tok = 0;
  int paramIdx = -1;
  do {
    tok = strtok(0, GCODE_PARAM_SEPERATOR);
    params[++paramIdx] = tok;
  }
  while (tok != 0);

#ifdef DEBUG_PRINTS
  printCmd(cmd, (char**)&params);
#endif
  executeCmd(cmd, (char**)&params);
}


void printCmd(char* cmd, char** params) {
  Serial.print("Command: ");
  Serial.println(cmd);
  if (params[0] != 0) {
    Serial.println("Parameters:");
    for (int i = 0; i < MAX_GCODE_PARAMS; i++)
    {
      char* p = params[i];
      if (p == 0)
        return;
      Serial.println(p);
    }

    Serial.println();
  }
}

void executeCmd(char* cmd, char** params) {
  char* errCheck;
  uint8_t cmdCode = strtol(&cmd[1],&errCheck,10);
  if(&cmd[1] == errCheck){
    SEND_ERROR(ERROR_UNKNOWN_CODE);
    return;
  }

  switch (cmd[0]) {
  case 'G':
    executeGCode(cmdCode, params);
    break;
  case 'M':
    executeMCode(cmdCode, params);
    break;
  default:
    SEND_ERROR(ERROR_UNKNOWN_CODE);
  }
}

bool executeGCode(int code, char** params) {
  switch (code) {
  case 0:
    executeG0(params);
    break;
  case 28:
    executeG28(params);
    break;
  case 90:
    absolutePositioning = true;
    SEND_NOERROR;
    return true;
  case 91:
    absolutePositioning = false;
    SEND_NOERROR;
    return true;
  default:
    SEND_ERROR(ERROR_UNKNOWN_CODE);
    return false;
  }
}
void executeMCode(int code, char** params) {
  switch (code) {
  case 3: // PEN_UP
    hw_ctrl_set_drawing(false);
    SEND_NOERROR;
    break;
  case 4: // PEN_DOWN
    hw_ctrl_set_drawing(true);
    SEND_NOERROR;
    break;
  case 5:
    executeM5(params);
    break;
  case 7:
    executeM7();
    break;
  case 8:
    executeM8();
    break;
  default:
    SEND_ERROR(ERROR_UNKNOWN_CODE);
  }
}



// CALIBRATE
boolean executeM5(char** params){
  // Calibrate origin
  if (params[0][0] != 'B' ||
    params[1][0] != 'L' ||
    params[2][0] != 'R') {
    SEND_ERROR(ERROR_INVALID_PARAM);
    return false;
  }
  float base;
  float l1, l2;
  char* errCheck;
  base = strtod(&params[0][1], &errCheck);
  if (&params[0][1] == errCheck)
  {
    SEND_ERROR(ERROR_INVALID_PARAM);
    return false;
  }
  l1 = strtod(&params[1][1], &errCheck);
  if (&params[1][1] == errCheck)
  {
    SEND_ERROR(ERROR_INVALID_PARAM);
    return false;
  }
  l2 = strtod(&params[2][1], &errCheck);
  if (&params[2][1] == errCheck)
  {
    SEND_ERROR(ERROR_INVALID_PARAM);
    return false;
  }

  if(hw_ctrl_calibrate(base, l1, l2))
    SEND_NOERROR;
  else
    SEND_ERROR(ERROR_CALIB_FAILED);
  return true;
}

// HOME
boolean executeG28(char** params){

    if(!hw_ctrl_is_calibrated())
    {
      SEND_ERROR(ERROR_CALIB_FIRST);
      return false;
    }

  hw_ctrl_execute_motion(HOME_POS_X,HOME_POS_Y);
  SEND_NOERROR;
  return true;
}

// MOVE TO
boolean executeG0(char** params){

  if(!hw_ctrl_is_calibrated())
  {
    SEND_ERROR(ERROR_CALIB_FIRST);
    return false;
  }
  char* errCheck;
  float X,Y;
  int speed;


  X = hw_state.coordinate_x_start;
  Y = hw_state.coordinate_y_start;


  for (int i = 0; i < MAX_GCODE_PARAMS; i++)
  {
    char* p = params[i];

    if (p == 0)
      break;
    switch (p[0]) {
    case 'X':
      if(absolutePositioning)
        X= strtod(&p[1], &errCheck);
      else
        X+= strtod(&p[1], &errCheck);

      if (&params[0][1] == errCheck)
      {
        SEND_ERROR(ERROR_INVALID_PARAM);
        return false;
      }
      break;
    case 'Y':
      if(absolutePositioning)
        Y= strtod(&p[1], &errCheck);
      else
        Y+= strtod(&p[1], &errCheck);
      if (&params[0][1] == errCheck)
      {
        SEND_ERROR(ERROR_INVALID_PARAM);
        return false;
      }
      break;
    case 'F':
      speed = strtol(&p[1], &errCheck, 10);
      if (&params[0][1] == errCheck)
      {
        SEND_ERROR(ERROR_INVALID_PARAM);
        return false;
      }
      hw_ctrl_set_speed(speed);
      break;
    }
  }

  hw_ctrl_execute_motion(X, Y);
  SEND_NOERROR;
  return true;
}
void executeM7()
{
    Serial.println("System status");
    Serial.println("Hardware state");
    Serial.println("MotionTime: " + String(hw_state.motor_motion_time));
    Serial.println("DeltaL: " + String(hw_state.motor_pos_left_delta));
    Serial.println("DeltaR: " + String(hw_state.motor_pos_right_delta));
    Serial.println("Speed: " + String(hw_state.motor_speed_mm_per_min));
    Serial.println("GoalL: " + String(hw_state.motor_pos_left_goal));
    Serial.println("GoalR: " + String(hw_state.motor_pos_right_goal));
    Serial.println("StartL: " + String(hw_state.motor_pos_left_start));
    Serial.println("StartR: " + String(hw_state.motor_pos_right_start));
    Serial.println("GoalX: " + String(hw_state.coordinate_x_goal));
    Serial.println("GoalY: " + String(hw_state.coordinate_y_goal));
    Serial.println("StartX: " + String(hw_state.coordinate_x_start));
    Serial.println("StartY: " + String(hw_state.coordinate_y_start));
}
void executeM8(){
    float pX,pY;
    hw_ctrl_convert_length_to_point(STEPS_TO_LENGTH(hw_state.motor_pos_left_start), STEPS_TO_LENGTH(hw_state.motor_pos_right_start), &pX, &pY);
    char strX[10];
    char strY[10];
    dtostrf(pX, 5, 1, strX);
    dtostrf(pY, 5, 1, strY);
    Serial.println(String(strX) + String(" ") + String(strY));
}
