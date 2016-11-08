
// Uncomment to enable debug prints
#define DEBUG_PRINTS
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

// Defines the home position
#define HOME_POS_X 0.1
#define HOME_POS_Y 0.1


#include "fifo.h"
#include "hardwareCtrl.h"


// Variables for serial communication
fifo_t buffer;

void setup() {

  // Start serial port
  Serial.begin(9600);
  Serial.setTimeout(100);
  
  // Initialize ringbuffer
  buffer = fifo_init(CMD_RING_BUFFER_SIZE + 1);
  hw_ctrl_init();
  Serial.println("System initialized..");


}

void loop() {
  // Always get bytes from buffer to avoid buffer overflows on serial port
  recieveSerialInput();
  
  // Wait for last command
  if (!hw_ctrl_final_pos_reached())
    return;

  processSerialInput();
}



/**
   Processes incoming G-Code commands
*/
void processSerialInput() {

  size_t bytes_used = fifo_get_used_bytes(&buffer);
  if (bytes_used == 0)
    return;

  // Maximmum available bytes
  fifo_data_t tmpBuff[MAX_CMD_SIZE];

  size_t sz = fifo_get_until(&buffer, tmpBuff, GCODE_COMMAND_SEPERATOR, MAX_CMD_SIZE);
  if (sz == 0)
  {
#ifdef DEBUG_PRINTS
    if (bytes_used > MAX_CMD_SIZE)
      Serial.println("No delimiter found, but enough data available! This should not happen.");
#endif
    return;
  }

  tmpBuff[sz - 1] = 0;    // Replace seperator with string termination

  // Split data into cmd and parameters
  char* params[MAX_GCODE_PARAMS];
  // First token is command
  char* cmd = strtok(tmpBuff, GCODE_PARAM_SEPERATOR);

  // extract params
  char* tok = 0;
  int paramIdx = -1;
  do {
    tok = strtok(0, GCODE_PARAM_SEPERATOR);
    params[++paramIdx] = tok;
  } while (tok != 0);

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
#ifdef DEBUG_PRINTS
    Serial.println("Cmd number missing: " + String(cmdCode));
#endif
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
      Serial.print("Invalid command: " + String(cmdCode));
  }
}

void executeGCode(int code, char** params) {
  char* errCheck;
  float tmpf;
  int tmpi;
  switch (code) {
    case 0: {      // MOVE TO
        for (int i = 0; i < MAX_GCODE_PARAMS; i++)
        {
          char* p = params[i];

          if (p == 0)
            break;
          switch (p[0]) {
            case 'X':
              tmpf= strtod(&p[1], &errCheck);
              if (&params[0][1] == errCheck)
              {
#ifdef DEBUG_PRINTS
                Serial.println("Invalid coordinate");
#endif
                return;
              }
              hw_ctrl_move_to_x(tmpf);
              break;
            case 'Y':
              tmpf = strtod(&p[1], &errCheck);
              if (&params[0][1] == errCheck)
              {
#ifdef DEBUG_PRINTS
                Serial.println("Invalid coordinate");
#endif
                return;
              }
              hw_ctrl_move_to_y(tmpf);
              break;
            case 'F':
              tmpi = strtol(&p[1], &errCheck, 10);
              if (&params[0][1] == errCheck)
              {
#ifdef DEBUG_PRINTS
                Serial.println("Invalid speed");
#endif
                return;
              }
              hw_ctrl_set_speed(tmpi);
          }
        }
        hw_ctrl_update_cord_length();
        break;
      }
    case 28:  // HOME
      hw_ctrl_move_to_x(HOME_POS_X);
      hw_ctrl_move_to_y(HOME_POS_Y);
      hw_ctrl_update_cord_length();
      break;
#ifdef DEBUG_PRINTS
    default:
      Serial.println("Unknown g code: " + String(code));
#endif  
  }
}
void executeMCode(int code, char** params) {
  switch (code) {
    case 5: {      // Calibrate origin
        if (params[0][0] != 'B' ||
            params[1][0] != 'L' ||
            params[2][0] != 'K') {
          Serial.println("Invalid calibration command");
          return;
        }
        float base;
        float l1, l2;
        char* errCheck;
        base = strtod(&params[0][1], &errCheck);
        if (&params[0][1] == errCheck)
        {
#ifdef DEBUG_PRINTS
          Serial.println("Invalid baselength");
          #endif
          return;
        }
        l1 = strtod(&params[1][1], &errCheck);
        if (&params[1][1] == errCheck)
        {
#ifdef DEBUG_PRINTS
          Serial.println("Invalid l1");
          #endif
          return;
        }
        l2 = strtod(&params[2][1], &errCheck);
        if (&params[2][1] == errCheck)
        {
#ifdef DEBUG_PRINTS
          Serial.println("Invalid l2");
          #endif
          return;
        }

        hw_ctrl_calibrate_home(base, l1, l2);
        
#ifdef DEBUG_PRINTS
        Serial.println("Calibrated: " + String(hw_state.calibration_x) + " " + String(hw_state.calibration_y));
  #endif
        break;
      }
    case 3: // PEN_UP
      hw_ctrl_set_servo_state(false);
      break;
    case 4: // PEN_DOWN
      hw_ctrl_set_servo_state(true);
      break;
    default:
      Serial.println("Unknown g code: " + String(code));
  }
}


void recieveSerialInput() {
  if (Serial.available()) {
    size_t sz = min(Serial.available(), fifo_get_free_bytes(&buffer));

    // Buffer full ?
    if (sz == 0) {
#ifdef DEBUG_PRINTS
      Serial.println("Buffer full! Please wait!");
#endif
      return;
    }

    char tmp[sz];

    size_t actualSz = Serial.readBytes(tmp, sz);
    size_t storedSz = fifo_put_data(&buffer, tmp, actualSz);

    //Serial.println("Stored " + String(storedSz) + " of " + String(actualSz) + " bytes.");
  }
}

