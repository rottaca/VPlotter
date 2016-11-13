#ifndef _H_HARDWARE_CTRL_
#define _H_HARDWARE_CTRL_

#include <stdlib.h>
#include <stddef.h>
#include <string.h>
#include <stdio.h>

#include <Arduino.h>
#include <EEPROM.h>
#include "TimerOne.h"

// Use microstepping multiplier
#define MOTOR_MICRO_STEPPING 32
// Length of cord per revolution in mm
#define CORD_LENGTH_PER_REVOLUTION M_PI*5.0f
// Full steps necessary for a single rotation
#define STEPPER_STEPS_PER_REVOLUTION 200
// Speed of timer0, used for servo positioning and stepper control
#define TIMER1_US_PER_INTERRUPT 500
// Default speed for stepper motors in microseconds per microstep
#define DEFAULT_SPEED 500         // mm/min

// Pin layout
#define PIN_DIR_LEFT 2
#define PIN_DIR_RIGHT 6
#define PIN_STEP_LEFT 3
#define PIN_STEP_RIGHT 7
#define PIN_SERVO 9

// Use -1 or 1 to invert stepper direction
#define INVERT_STEPPER_LEFT 1
#define INVERT_STEPPER_RIGHT 1

// Valid drawing area, Movements are clamped to this area
#define X_MIN 10
#define X_MAX 600
#define Y_MIN 10
#define Y_MAX 1100

// Defines the home position
#define HOME_POS_X 100
#define HOME_POS_Y 100

// Servo setup
#define SERVO_US_FULL_PHASE 20000   // 20ms
#define SERVO_US_POS_UP 1800        // 1.8ms
#define SERVO_US_POS_DOWN 1000      // 1ms

// Macro to convert cord length into steps
#define LENGTH_TO_STEPS(l) ((l)/(CORD_LENGTH_PER_REVOLUTION) * STEPPER_STEPS_PER_REVOLUTION * MOTOR_MICRO_STEPPING )
#define STEPS_TO_LENGTH(s) ((s) *CORD_LENGTH_PER_REVOLUTION /(STEPPER_STEPS_PER_REVOLUTION * MOTOR_MICRO_STEPPING))
#define CLAMP(V,MIN,MAX) ((V)<(MIN)?(MIN):((V)>(MAX)?(MAX):(V)))

// Struct that contains all hardware data
struct hardware_state_t
{
  boolean calibrated;
  // Baselength between both stepper motors
  float base;
  // Position used for calibration
  float calibration_x;
  float calibration_y;

  // Speed: mm/min
  uint32_t motor_speed_mm_per_min;

  // time in us
  uint32_t motor_motion_time;
  uint32_t motor_motion_time_since_start;

  int32_t motor_pos_left_delta;
  int32_t motor_pos_right_delta;

  int32_t motor_pos_left_delta_last;
  int32_t motor_pos_right_delta_last;

  // motor position where motion started
  uint32_t motor_pos_left_start;
  uint32_t motor_pos_right_start;

  // Motor position to reach
  uint32_t motor_pos_left_goal;
  uint32_t motor_pos_right_goal;

  // XY-position to reach
  float coordinate_x_goal;
  float coordinate_y_goal;

  // XY-position where motion started
  float coordinate_x_start;
  float coordinate_y_start;

  // Time since last servo signal
  uint32_t servo_signal_length_us;
  // Servo up-time in microseconds -> Defines servo position
  uint32_t timer0_us_since_servo_period;
};

struct hardware_state_t hw_state;

// FWD declarationR
void hw_ctrl_initR();
void hw_ctrl_move_to(float x, float y);
void hw_ctrl_add(float x, float y);
void hw_ctrl_set_speed(uint32_t s);
void hw_ctrl_set_drawing(bool drawing);
bool hw_ctrl_calibrate(float base, float lengthL, float lengthR);
void hw_ctrl_timer_callback();
void hw_ctrl_convert_length_to_point(float L, float R, float* x, float* y);
void hw_ctrl_execute_motion(float x, float y);

inline bool hw_ctrl_is_moving();
inline bool hw_ctrl_is_calibrated();

void hw_ctrl_init()
{
  memset(&hw_state,0,sizeof(struct hardware_state_t));

  hw_state.motor_speed_mm_per_min = DEFAULT_SPEED;

  // Initialize hardware
  pinMode(PIN_DIR_LEFT, OUTPUT);
  pinMode(PIN_DIR_RIGHT, OUTPUT);
  pinMode(PIN_STEP_LEFT, OUTPUT);
  pinMode(PIN_STEP_RIGHT, OUTPUT);
  pinMode(PIN_SERVO, OUTPUT);

  // Lift the pen
  hw_ctrl_set_drawing(false);

  // Start timer for motor control
  Timer1.initialize(TIMER1_US_PER_INTERRUPT);
  Timer1.attachInterrupt(hw_ctrl_timer_callback);

}

void hw_ctrl_set_speed(uint32_t s)
{
  if(hw_ctrl_is_moving())
    return;

  hw_state.motor_speed_mm_per_min = s;
}

void hw_ctrl_set_drawing(bool drawing)
{
  if(hw_ctrl_is_moving())
    return;
    hw_state.servo_signal_length_us = drawing?SERVO_US_POS_DOWN:SERVO_US_POS_UP;
}

bool hw_ctrl_calibrate(float base, float lengthL, float lengthR)
{
  hw_state.calibrated = false;

  if(base > lengthL+lengthR)
  {
    Serial.println("Calibration failed. Invalid length's.");
    return false;
  }

  hw_state.base = base;
  hw_state.calibration_x = (base * base + lengthL * lengthL - lengthR * lengthR) / (2 * base);
  hw_state.calibration_y = sqrt(lengthR * lengthR - (hw_state.base - hw_state.calibration_x) * (hw_state.base - hw_state.calibration_x));

  if(hw_state.calibration_x < X_MIN || hw_state.calibration_x > X_MAX ||
     hw_state.calibration_y < Y_MIN || hw_state.calibration_y > Y_MAX)
  {
    Serial.println("Calibration failed. Position outside of drawing area!");
    return false;
  }
  hw_state.motor_pos_left_start = LENGTH_TO_STEPS(lengthL);
  hw_state.motor_pos_right_start =  LENGTH_TO_STEPS(lengthR);
  hw_state.motor_pos_left_goal = hw_state.motor_pos_left_start;
  hw_state.motor_pos_right_goal = hw_state.motor_pos_right_start;
  hw_state.coordinate_x_start = hw_state.calibration_x;
  hw_state.coordinate_y_start = hw_state.calibration_y;
  hw_state.coordinate_x_goal = hw_state.calibration_x;
  hw_state.coordinate_y_goal = hw_state.calibration_y;
  hw_state.motor_pos_left_delta = 0;
  hw_state.motor_pos_right_delta = 0;
  hw_state.calibrated = true;

  Serial.println(String((int)hw_state.calibration_x) + String(" ") + String((int)hw_state.calibration_y));
  Serial.println(String(hw_state.motor_pos_left_start) + String(" ") + String(hw_state.motor_pos_right_start));
}


void hw_ctrl_execute_motion(float x_input, float y_input) {

  if(!hw_ctrl_is_calibrated() || hw_ctrl_is_moving())
    return;

  float x,y;
  x = CLAMP(x_input,X_MIN,X_MAX);
  y = CLAMP(y_input,Y_MIN,Y_MAX);

  if(x != x_input || y != y_input)
  {
  }

     Serial.println(String("Warning! Coordinates clamped: ") +
  String((int)x_input) + String("->") + String((int)x) + String(" and ") +
  String((int)y_input) + String("->") + String((int)y));

  float l1 = sqrt(x * x + y * y);
  float l2 = sqrt((hw_state.base - x) * (hw_state.base - x) + y * y);

  hw_state.coordinate_x_goal = x;
  hw_state.coordinate_y_goal = y;
  hw_state.motor_pos_left_goal = LENGTH_TO_STEPS(l1);
  hw_state.motor_pos_right_goal = LENGTH_TO_STEPS(l2);
  hw_state.motor_pos_left_delta = hw_state.motor_pos_left_goal-hw_state.motor_pos_left_start;
  hw_state.motor_pos_right_delta = hw_state.motor_pos_right_goal-hw_state.motor_pos_right_start;
  hw_state.motor_pos_left_delta_last = 0;
  hw_state.motor_pos_right_delta_last = 0;

  // Set direction

  // Both motors move in same direction ?
  if(hw_state.motor_pos_left_delta*hw_state.motor_pos_right_delta > 0){
    if(hw_state.motor_pos_right_delta*INVERT_STEPPER_RIGHT < 0)
        digitalWrite(PIN_DIR_RIGHT, 1);
    else
        digitalWrite(PIN_DIR_RIGHT, 0);

    // Set direction
    if(hw_state.motor_pos_left_delta*INVERT_STEPPER_LEFT < 0)
        digitalWrite(PIN_DIR_LEFT, 1);
    else
        digitalWrite(PIN_DIR_LEFT, 0);
  }else{
    if(hw_state.motor_pos_right_delta*INVERT_STEPPER_RIGHT < 0)
        digitalWrite(PIN_DIR_RIGHT, 0);
    else
        digitalWrite(PIN_DIR_RIGHT, 1);

    // Set direction
    if(hw_state.motor_pos_left_delta*INVERT_STEPPER_LEFT < 0)
        digitalWrite(PIN_DIR_LEFT, 0);
    else
        digitalWrite(PIN_DIR_LEFT, 1);
  }

  // Compute time for full motion
  float dx = hw_state.coordinate_x_goal-hw_state.coordinate_x_start;
  float dy = hw_state.coordinate_y_goal-hw_state.coordinate_y_start;
  hw_state.motor_motion_time_since_start = 0;
  hw_state.motor_motion_time = sqrt(dx*dx+dy*dy)*60000000/hw_state.motor_speed_mm_per_min;
}

void hw_ctrl_timer_callback() {

  // Control servo first
  hw_state.timer0_us_since_servo_period += TIMER1_US_PER_INTERRUPT;
  hw_state.motor_motion_time_since_start += TIMER1_US_PER_INTERRUPT;

  if (hw_state.timer0_us_since_servo_period >= SERVO_US_FULL_PHASE) {
    hw_state.timer0_us_since_servo_period = 0;
    digitalWrite(PIN_SERVO, 1);
  } else if (hw_state.timer0_us_since_servo_period > hw_state.servo_signal_length_us) {
    digitalWrite(PIN_SERVO, 0);
  }

  // No motion set ?
  if(!hw_ctrl_is_moving() || !hw_ctrl_is_calibrated())
    return;

  float t = (float)hw_state.motor_motion_time_since_start/hw_state.motor_motion_time;

  int32_t dL1 = floor(hw_state.motor_pos_left_delta*t);
  int32_t stepsL = abs(dL1 - hw_state.motor_pos_left_delta_last);
  hw_state.motor_pos_left_delta_last = dL1;

  int32_t dR1 = floor(hw_state.motor_pos_right_delta*t);
  int32_t stepsR = abs(dR1 - hw_state.motor_pos_right_delta_last);
  hw_state.motor_pos_right_delta_last = dR1;

  for(int i=0; i < stepsL;i++){
    digitalWrite(PIN_STEP_LEFT, 1);
    digitalWrite(PIN_STEP_LEFT, 0);
  }

  for(int i=0; i < stepsR;i++){
    digitalWrite(PIN_STEP_RIGHT, 1);
    digitalWrite(PIN_STEP_RIGHT, 0);
  }

  // Motion finished ?
  if(hw_state.motor_motion_time_since_start>=hw_state.motor_motion_time )
  {
      hw_state.motor_pos_left_start = hw_state.motor_pos_left_goal;
      hw_state.motor_pos_right_start = hw_state.motor_pos_right_goal;
      hw_state.coordinate_x_start = hw_state.coordinate_x_goal;
      hw_state.coordinate_y_start = hw_state.coordinate_y_goal;
      hw_state.motor_motion_time = 0;
  }
}

inline bool hw_ctrl_is_moving() {
  return hw_state.motor_motion_time > 0;
}

inline bool hw_ctrl_is_calibrated()
{
  return hw_state.calibrated;
}

void hw_ctrl_convert_length_to_point(float L, float R, float* x, float* y)
{
  if(!hw_ctrl_is_calibrated())
  {
        *x = NAN;
        *y = NAN;
        return;
  }
  *x = (hw_state.base * hw_state.base + L * L - R * R) / (2 * hw_state.base);
  *y = sqrt(R * R - (hw_state.base - *x) * (hw_state.base - *x));
}


#endif
