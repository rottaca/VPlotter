#include <Arduino.h>
#include "settings.h"
#include "macros.h"
#include "hardwareCtrl.h"

// Forward declarations
void processSerialInput();
void printCmd(char* cmd, char** params);
void executeCmd(char* cmd, char** params);
bool executeGCode(int code, char** params) ;
void executeMCode(int code, char** params) ;
size_t recieveSerialInput(char* data);
void processSerialInput(char* data);

boolean executeG0(char** params);
boolean executeG28(char** params);
boolean executeM6(char** params);
boolean executeM5(char** params);
void executeM8();
void executeM7();

// Variables for serial communication
char cmdBuffer[CMD_RING_BUFFER_SIZE][MAX_CMD_SIZE+1];
uint16_t writeIdx;
uint16_t readIdx;
#define BUFFER_NEXT(p) ((p+1)%CMD_RING_BUFFER_SIZE)
#define BUFFER_FULL() (BUFFER_NEXT(writeIdx) == readIdx)
#define BUFFER_EMPTY() (readIdx == writeIdx)

// Systemstate
boolean absolutePositioning = true;

void setup() {

  // Start serial port
  Serial.begin(9600);
  Serial.setTimeout(SERIAL_TIMEOUT);

  // Init pointers on ringbuffer
  writeIdx = 0;
  readIdx = writeIdx;

  hw_ctrl_init();
  Serial.println("System initialized");
  Serial.println("Version 1.0");
}

void loop() {

  // Buffer not full?
  if( !BUFFER_FULL() ){
    // Get cmd
    if(recieveSerialInput(cmdBuffer[writeIdx])> 0){
      //Serial.print("Recieved:");
      //Serial.println(cmdBuffer[writeIdx]);

      // Position filled with data, go to next
      writeIdx = BUFFER_NEXT(writeIdx);
    }
  }else{
        delay(100);
        SEND_BUSY;
  }

  if(hw_ctrl_is_busy()||BUFFER_EMPTY())
    return;

  // Execute new command and toto next
  //Serial.print("Execute:");
  //Serial.println(cmdBuffer[readIdx]);
  processSerialInput(cmdBuffer[readIdx]);
  readIdx = BUFFER_NEXT(readIdx);
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
    // Skip comments
  case ';':
    SEND_NOERROR;
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
  case 3: // DOWN
    if(hw_ctrl_set_drawing(true))
      SEND_NOERROR;
    else
      SEND_ERROR(ERROR_CALIB_FIRST);
    break;
  case 4: // UP
    if(hw_ctrl_set_drawing(false))
      SEND_NOERROR;
    else
      SEND_ERROR(ERROR_CALIB_FIRST);
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
  bool positionChanged = false;

  hw_ctrl_convert_length_to_point(STEPS_TO_LENGTH(hw_state.motor_pos[STP_LEFT]),
    STEPS_TO_LENGTH(hw_state.motor_pos[STP_RIGHT]),
    &X, &Y);

  for (int i = 0; i < MAX_GCODE_PARAMS; i++)
  {
    char* p = params[i];

    if (p == 0)
      break;
    switch (p[0]) {
    case 'X':{
      float tmp;
      if(absolutePositioning)
        tmp = strtod(&p[1], &errCheck);
      else
        tmp = X + strtod(&p[1], &errCheck);

      if (&p[1] == errCheck)
      {
        SEND_ERROR(ERROR_INVALID_PARAM);
        return false;
      }
      if(abs(X-tmp) > 0.0001){
        X = tmp;
        positionChanged = true;
      }
      break;
    }
    case 'Y':{
      float tmp;
      if(absolutePositioning)
        tmp = strtod(&p[1], &errCheck);
      else
        tmp = Y + strtod(&p[1], &errCheck);

      if (&p[1] == errCheck)
      {
        SEND_ERROR(ERROR_INVALID_PARAM);
        return false;
      }
      if(abs(Y-tmp) > 0.0001){
        positionChanged = true;
        Y = tmp;
      }
      break;
    }
    case 'F':{
      uint8_t tmp;
      tmp = strtol(&p[1], &errCheck, 10);
      if(&p[0] == errCheck){
        SEND_ERROR(ERROR_INVALID_PARAM);
        return false;
      }
      hw_ctrl_set_speed_devider(tmp);
    }
  }
}
if(positionChanged)
  hw_ctrl_execute_motion(X, Y);

  SEND_NOERROR;
  return true;
}
void executeM7()
{
    Serial.println("State: " + String(hw_state.state));
    Serial.println("Base: " + String((int)hw_state.base));
    float x,y;
    hw_ctrl_convert_length_to_point(STEPS_TO_LENGTH(hw_state.motor_pos[STP_LEFT]),
      STEPS_TO_LENGTH(hw_state.motor_pos[STP_RIGHT]),&x, &y);

    Serial.println("Position X: " + String((int)x));
    Serial.println("Position Y: " + String((int)y));
    hw_ctrl_convert_length_to_point(STEPS_TO_LENGTH(hw_state.motor_pos_target[STP_LEFT]),
      STEPS_TO_LENGTH(hw_state.motor_pos_target[STP_RIGHT]),&x, &y);

    Serial.println("Target X: " + String((int)x));
    Serial.println("Target Y: " + String((int)y));
    Serial.println("Position L: " + String(hw_state.motor_pos[STP_LEFT]));
    Serial.println("Target L: " + String(hw_state.motor_pos_target[STP_LEFT]));
    Serial.println("Delta L: " + String(hw_state.dSteps[STP_LEFT]));
    Serial.println("Position R: " + String(hw_state.motor_pos[STP_RIGHT]));
    Serial.println("TargetR: " + String(hw_state.motor_pos_target[STP_RIGHT]));
    Serial.println("Delta R: " + String(hw_state.dSteps[STP_RIGHT]));
    Serial.println("S R: " + String(hw_state.s[STP_RIGHT]));
    Serial.println("S L: " + String(hw_state.s[STP_LEFT]));
    Serial.println("Bufferusage: " + String((writeIdx-readIdx)%CMD_RING_BUFFER_SIZE) + "/" + String(CMD_RING_BUFFER_SIZE));
    char tmp[10];
    dtostrf(LENGTH_TO_STEPS(1), 9, 3, tmp );
    Serial.println(String("L2S: ") + String(tmp));

    dtostrf(STEPS_TO_LENGTH(1), 9, 8, tmp );
    Serial.println(String("S2L: ") + String(tmp));

}
void executeM8(){
  float x,y;
  hw_ctrl_convert_length_to_point(STEPS_TO_LENGTH(hw_state.motor_pos[STP_LEFT]),
    STEPS_TO_LENGTH(hw_state.motor_pos[STP_RIGHT]),&x, &y);

  char strX[7], strY[7];
  dtostrf(x, 6, 2, strX);
  dtostrf(y, 6, 2, strY);

  SEND_NOERROR;
  Serial.println(String(strX) + String(" ") + String(strY));
}
