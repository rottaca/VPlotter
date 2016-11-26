#ifndef _H_SETTINGS_
#define _H_SETTINGS_

// Uncomment to enable debug prints
//#define DEBUG_PRINTS

// Size of the ringbuffer that contains all commands
#define CMD_RING_BUFFER_SIZE 20
// Size of the data that is read as a single piece
#define MAX_CMD_SIZE 32
// Maximum number of gcode parameters
#define MAX_GCODE_PARAMS 5
// Seperator for gcode parameters
#define GCODE_PARAM_SEPERATOR " "
// Seperator for gcode parameters
#define GCODE_COMMAND_SEPERATOR '\n'
// Timeout for serial read
#define SERIAL_TIMEOUT 100

// Microstepping multiplier
#define MICRO_STEPPING 32
// Full steps necessary for a single rotation
#define STEPS_PER_REVOLUTION 200
// Length of cord per revolution in mm
#define DIST_PER_REVOLUTION 50
// Speed of timer1, used for servo positioning and stepper control
#define TIMER1_US_PER_INTERRUPT 50
// Speed devider to slowdown movements
#define SPEED_DIVIDER 1

// Invert stepper direction
#define INVERT_STEPPER_LEFT -1
#define INVERT_STEPPER_RIGHT 1

// Pin layout
#define PIN_DIR_LEFT 6
#define PIN_DIR_RIGHT 2
#define PIN_STEP_LEFT 7
#define PIN_STEP_RIGHT 3
#define PIN_SERVO 9
#define PIN_LED 13

// Valid drawing area, Movements are clamped to this area
#define X_MIN 10
#define X_MAX 800
#define Y_MIN 10
#define Y_MAX 1200

// Defines the home position
#define HOME_POS_X 300
#define HOME_POS_Y 300

// Servo setup
#define SERVO_US_FULL_PHASE 20000   // 20ms
// drawing position
#define SERVO_US_PEN_DOWN 1600     // 1.6ms
// non-drawing position
#define SERVO_US_PEN_UP 1150      // 1ms
// Time for servo to reach position
#define SERVO_MOVE_DELAY 200000


#define LENGTH_TO_STEPS_CONST ((double)STEPS_PER_REVOLUTION * MICRO_STEPPING)/(DIST_PER_REVOLUTION)
#define STEPS_TO_LENGTH_CONST ((double)DIST_PER_REVOLUTION/(STEPS_PER_REVOLUTION*MICRO_STEPPING))


#endif
