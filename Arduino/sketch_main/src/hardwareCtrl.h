#ifndef _H_HARDWARE_CTRL_
#define _H_HARDWARE_CTRL_

#include <stdlib.h>
#include <stddef.h>
#include <string.h>
#include <stdio.h>

#include <Arduino.h>
#include "TimerOne.h"
#include "settings.h"
#include "macros.h"

// Stepper names and cnt: 0,1,2
enum {STP_LEFT,STP_RIGHT,STP_CNT};

// States for hardware state machine
enum state_t{START,IDLE,MOVING,WAIT_SERVO};

// Struct that contains all hardware data
struct hardware_state_t
{
  // State of the hardware state machine
  state_t state;

  // Baselength between both stepper motors
  float base;

  // Variables for bresenham
  int32_t err;
  int32_t s[STP_CNT];
  int32_t dSteps[STP_CNT];

  // properties for speed control
  uint8_t skipped_loops;
  uint8_t loops_to_skip;
  // motor position
  int32_t motor_pos[STP_CNT];
  // target motor position
  int32_t motor_pos_target[STP_CNT];

  // Delay since last posiion change, used to wait a predefined time
  // until next cmd is executed
  uint32_t servo_move_delay;
  // Servo pulse -> Defines servo position
  uint32_t servo_signal_length_us;
  // Time since last servo pulse start
  uint32_t us_since_servo_period;
};

volatile struct hardware_state_t hw_state;

// FWD declarationR
void hw_ctrl_init();
void hw_ctrl_move_to(float x, float y);
void hw_ctrl_add(float x, float y);
void hw_ctrl_set_speed(uint32_t s);
bool hw_ctrl_set_drawing(bool drawing);
bool hw_ctrl_calibrate(float base, float lengthL, float lengthR);
void hw_ctrl_timer_callback();
void hw_ctrl_convert_length_to_point(float L, float R, float* x, float* y);
void hw_ctrl_convert_point_to_length(float x, float y, float*L, float*R);
void hw_ctrl_execute_motion(float x, float y);

inline bool hw_ctrl_is_busy();

void hw_ctrl_init()
{

  for(int i = 0; i < STP_CNT; i++){
    hw_state.dSteps[i] = 0;
    hw_state.motor_pos[i] = 0;
    hw_state.motor_pos_target[i] = 0;
    hw_state.s[i] = 0;
  }

  hw_state.base = 0;
  hw_state.err = 0;
  hw_state.us_since_servo_period = 0;
  hw_state.servo_signal_length_us = SERVO_US_PEN_UP;
  hw_state.servo_move_delay = 0;
  hw_state.state = START;
  hw_state.skipped_loops = 0;
  hw_state.loops_to_skip = SPEED_DIVIDER;

  // Initialize hardware
  pinMode(PIN_DIR_LEFT, OUTPUT);
  pinMode(PIN_DIR_RIGHT, OUTPUT);
  pinMode(PIN_STEP_LEFT, OUTPUT);
  pinMode(PIN_STEP_RIGHT, OUTPUT);
  pinMode(PIN_SERVO, OUTPUT);
  pinMode(PIN_LED,OUTPUT);


  // Start timer for motor control
  Timer1.initialize(TIMER1_US_PER_INTERRUPT);
  Timer1.attachInterrupt(hw_ctrl_timer_callback);

  delay(500);
  // test stepper moves
  for (size_t dir = 0; dir < 2; dir++) {
    digitalWrite(PIN_DIR_LEFT, INVERT_STEPPER_LEFT==1?dir:1-dir);
    digitalWrite(PIN_DIR_RIGHT, INVERT_STEPPER_RIGHT==1?dir:1-dir);

    for (size_t i = 0; i < STEPS_PER_REVOLUTION*MICRO_STEPPING/5; i++) {
      digitalWrite(PIN_STEP_LEFT, 1);
      digitalWrite(PIN_STEP_RIGHT, 1);
      delay(1);
      digitalWrite(PIN_STEP_LEFT, 0);
      digitalWrite(PIN_STEP_RIGHT, 0);
    }
  }
}

bool hw_ctrl_set_speed_devider(uint8_t div){
  if(hw_state.state != IDLE)
    return false;

  hw_state.loops_to_skip = CLAMP(div,1,255);
}

bool hw_ctrl_set_drawing(bool drawing)
{
  if(hw_state.state != IDLE)
    return false;

  uint32_t newPos = drawing?SERVO_US_PEN_DOWN:SERVO_US_PEN_UP;
  // Avoid unnecessary waits
  if(newPos == hw_state.servo_signal_length_us)
    return true;

  hw_state.servo_signal_length_us = newPos;
  hw_state.servo_move_delay = 0;
  hw_state.state = WAIT_SERVO;
  return true;
}

bool hw_ctrl_calibrate(float base, float lengthL, float lengthR)
{
  if(hw_state.state != IDLE && hw_state.state != START)
    return false;

  hw_state.state = START;

  if(base > lengthL+lengthR)
  {
    //Serial.println("Calibration failed. Invalid length's.");
    return false;
  }

  hw_state.base = base;
  float x = (base * base + lengthL * lengthL - lengthR * lengthR) / (2 * base);
  float y = sqrt(lengthR * lengthR - (base - x) * (base - x));

  if(x < X_MIN || x > X_MAX ||
     y < Y_MIN || y > Y_MAX)
  {
    return false;
  }

  hw_state.motor_pos[STP_LEFT] = LENGTH_TO_STEPS(lengthL);
  hw_state.motor_pos[STP_RIGHT] = LENGTH_TO_STEPS(lengthR);
  hw_state.motor_pos_target[STP_LEFT] = LENGTH_TO_STEPS(lengthL);
  hw_state.motor_pos_target[STP_RIGHT] = LENGTH_TO_STEPS(lengthR);
  hw_state.state = IDLE;
  return true;
}

void hw_ctrl_execute_motion(float x, float y) {

  if(hw_state.state != IDLE)
    return;

  x = CLAMP(x,X_MIN,X_MAX);
  y = CLAMP(y,Y_MIN,Y_MAX);

  float L,R;
  hw_ctrl_convert_point_to_length(x,y,&L,&R);

  hw_state.motor_pos_target[STP_LEFT] = LENGTH_TO_STEPS(L);
  hw_state.motor_pos_target[STP_RIGHT] = LENGTH_TO_STEPS(R);
  // Optimized and faster bresenham line algorithm
  // Source:http://members.chello.at/easyfilter/bresenham.html
  hw_state.dSteps[STP_LEFT] = abs(hw_state.motor_pos_target[STP_LEFT] - hw_state.motor_pos[STP_LEFT]);
  hw_state.dSteps[STP_RIGHT] = abs(hw_state.motor_pos_target[STP_RIGHT] - hw_state.motor_pos[STP_RIGHT]);
  hw_state.s[STP_LEFT]  = hw_state.motor_pos_target[STP_LEFT] > hw_state.motor_pos[STP_LEFT]?1:-1;
  hw_state.s[STP_RIGHT]  = hw_state.motor_pos_target[STP_RIGHT] > hw_state.motor_pos[STP_RIGHT]?1:-1;
  hw_state.err = hw_state.dSteps[STP_LEFT]-hw_state.dSteps[STP_RIGHT];

  // Set direction
  digitalWrite(PIN_DIR_RIGHT,(hw_state.s[STP_RIGHT]*INVERT_STEPPER_RIGHT)>0?1:0);
  digitalWrite(PIN_DIR_LEFT,(hw_state.s[STP_LEFT]*INVERT_STEPPER_LEFT)>0?1:0);

  // Start motion
  hw_state.state = MOVING;
}

void hw_ctrl_timer_callback() {

      // 128 us per timer overflow
      hw_state.us_since_servo_period += TIMER1_US_PER_INTERRUPT;

      if (hw_state.us_since_servo_period >= SERVO_US_FULL_PHASE) {
        hw_state.us_since_servo_period = 0;
        digitalWrite(PIN_SERVO, 1);
      } else if (hw_state.us_since_servo_period > hw_state.servo_signal_length_us) {
        digitalWrite(PIN_SERVO, 0);
      }

  switch (hw_state.state) {
    case MOVING:
    {
        if (hw_state.skipped_loops < hw_state.loops_to_skip) {
          hw_state.skipped_loops++;
          break;
        }

        hw_state.skipped_loops = 0;

        // Motion finished ?
        if(hw_state.motor_pos[STP_LEFT] == hw_state.motor_pos_target[STP_LEFT] &&
          hw_state.motor_pos[STP_RIGHT] == hw_state.motor_pos_target[STP_RIGHT])
        {
            // go to idle
            hw_state.state = IDLE;
            break;
        }
        // Optimized and faster bresenham line algorithm
        // Source:http://members.chello.at/easyfilter/bresenham.html
        int32_t e2 = 2*hw_state.err;
        if(e2 >= -hw_state.dSteps[STP_RIGHT]){
          hw_state.err -= hw_state.dSteps[STP_RIGHT];
          digitalWrite(PIN_STEP_LEFT, 1);
          hw_state.motor_pos[STP_LEFT] += hw_state.s[STP_LEFT];
        }
        if(e2 <= hw_state.dSteps[STP_LEFT]){
          hw_state.err += hw_state.dSteps[STP_LEFT];
          digitalWrite(PIN_STEP_RIGHT, 1);
          hw_state.motor_pos[STP_RIGHT] += hw_state.s[STP_RIGHT];
        }
        // Reset pins to zero
        digitalWrite(PIN_STEP_LEFT, 0);
        digitalWrite(PIN_STEP_RIGHT, 0);
    }
    break;
  case WAIT_SERVO:
    hw_state.servo_move_delay += TIMER1_US_PER_INTERRUPT;
    if(hw_state.servo_move_delay>= SERVO_MOVE_DELAY){
      hw_state.state = IDLE;
    }
    break;
  default:
      break;
  }
}

inline bool hw_ctrl_is_busy() {
  return hw_state.state != IDLE && hw_state.state != START;
}

inline bool hw_ctrl_is_calibrated()
{
  return hw_state.state != START;
}

void hw_ctrl_convert_length_to_point(float L, float R, float* x, float* y){

  if(hw_state.state == START)
  {
        *x = NAN;
        *y = NAN;
        return;
  }
  *x = (hw_state.base * hw_state.base + L * L - R * R) / (2 * hw_state.base);
  *y = sqrt(R * R - (hw_state.base - *x) * (hw_state.base - *x));
}

void hw_ctrl_convert_point_to_length(float x, float y, float* L, float* R){

  if(hw_state.state == START)
  {
        *L = NAN;
        *R = NAN;
        return;
  }
  *L = sqrt(x * x + y * y);
  *R = sqrt((hw_state.base - x) * (hw_state.base - x) + y * y);
}


#endif
