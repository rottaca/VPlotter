#ifndef _H_HARDWARE_CTRL_
#define _H_HARDWARE_CTRL_

#include <stdlib.h>
#include <stddef.h>
#include <string.h>
#include <stdio.h>

#include <Arduino.h>
#include "TimerOne.h"

// Use microstepping multiplier
#define MOTOR_MICRO_STEPPING 32
// Length of cord per revolution in mm
#define CORD_LENGTH_PER_REVOLUTION 31.4f
// Full steps necessary for a single rotation
#define STEPPER_STEPS_PER_REVOLUTION 360/1.8f
// Speed of timer0, used for servo positioning and stepper control
#define TIMER1_US_PER_INTERRUPT 1000
// Default speed for stepper motors in microseconds per microstep
#define DEFAULT_SPEED 1000         // mm/min

// Pin layout
#define PIN_DIR_LEFT 2
#define PIN_DIR_RIGHT 6
#define PIN_STEP_LEFT 3
#define PIN_STEP_RIGHT 7
#define PIN_SERVO 9

// Servo setup
#define SERVO_US_FULL_PHASE 20000   // 20ms
#define SERVO_US_POS_UP 1800        // 1.8ms
#define SERVO_US_POS_DOWN 1000      // 1ms

// Macro to convert cord length into steps
#define LENGTH_TO_STEPS(l) (l/ CORD_LENGTH_PER_REVOLUTION * STEPPER_STEPS_PER_REVOLUTION * MOTOR_MICRO_STEPPING )
#define STEPS_TO_LENGTH(s) (s * CORD_LENGTH_PER_REVOLUTION /(STEPPER_STEPS_PER_REVOLUTION * MOTOR_MICRO_STEPPING))

// Struct that contains all hardware data
struct hardware_state_t
{
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

// FWD declaration
void hw_ctrl_init();
void hw_ctrl_move_to_x(float x);
void hw_ctrl_move_to_y(float y);
void hw_ctrl_set_speed(int32_t f);
void hw_ctrl_set_servo_state(bool drawing);
void hw_ctrl_calibrate_home(float base, float length1, float length2);
bool hw_ctrl_final_pos_reached();
void hw_ctrl_timer_callback();
void hw_ctrl_convert_length_to_pos();

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
  hw_ctrl_set_servo_state(false);

  // Start timer for motor control
  Timer1.initialize(TIMER1_US_PER_INTERRUPT);
  Timer1.attachInterrupt(hw_ctrl_timer_callback);

  // Use default calibration with a baselength of 1 m
  hw_ctrl_calibrate_home(1000, 100, 1000);
}

void hw_ctrl_move_to_x(float x)
{
#ifdef DEBUG_PRINTS
  //Serial.println("Move To X: " + String(x));
#endif
  hw_state.coordinate_x_goal = x;
}

void hw_ctrl_move_to_y(float y)
{
#ifdef DEBUG_PRINTS
  //Serial.println("Move To Y: " + String(y));
#endif
  hw_state.coordinate_y_goal = y;
}

void hw_ctrl_set_speed(int32_t f)
{
#ifdef DEBUG_PRINTS
  Serial.println("Set Speed: " + String(f));
#endif
  hw_state.motor_speed_mm_per_min = f;
}

void hw_ctrl_set_servo_state(bool drawing)
{
  if (drawing) {
    hw_state.servo_signal_length_us = SERVO_US_POS_DOWN;
  }
  else {
    hw_state.servo_signal_length_us = SERVO_US_POS_UP;
  }
}

void hw_ctrl_calibrate_home(float base, float length1, float length2)
{
  hw_state.base = base;
  // TODO Remove origin ?!
  hw_state.calibration_x = (base * base + length1 * length1 - length2 * length2) / (2 * base);
  hw_state.calibration_y = sqrt(length2 * length2 - (hw_state.base - hw_state.calibration_x) * (hw_state.base - hw_state.calibration_x));
  
  hw_state.motor_pos_left_start = LENGTH_TO_STEPS(length1);
  hw_state.motor_pos_right_start =  LENGTH_TO_STEPS(length2);
  hw_state.motor_pos_left_goal = hw_state.motor_pos_left_start;
  hw_state.motor_pos_right_goal = hw_state.motor_pos_right_start;
  hw_state.coordinate_x_start = hw_state.calibration_x;
  hw_state.coordinate_y_start = hw_state.calibration_y;
  hw_state.coordinate_x_goal = 0;
  hw_state.coordinate_y_goal = 0;
#ifdef DEBUG_PRINTS
  Serial.println("LengthL: " + String(hw_state.motor_pos_left_start));
  Serial.println("LengthR: " + String(hw_state.motor_pos_right_start));
#endif
}

void hw_ctrl_update_cord_length() {
  
  float x = hw_state.coordinate_x_goal;// + hw_state.calibration_x;
  float y = hw_state.coordinate_y_goal;// + hw_state.calibration_x;

  float l1 = sqrt(x * x + y * y);
  float l2 = sqrt((hw_state.base - x) * (hw_state.base - x) + y * y);

  hw_state.motor_pos_left_goal = LENGTH_TO_STEPS(l1);
  hw_state.motor_pos_right_goal = LENGTH_TO_STEPS(l2);
  hw_state.motor_pos_left_delta = hw_state.motor_pos_left_goal-hw_state.motor_pos_left_start;
  hw_state.motor_pos_right_delta = hw_state.motor_pos_right_goal-hw_state.motor_pos_right_start;
  hw_state.motor_pos_left_delta_last = 0;
  hw_state.motor_pos_right_delta_last = 0;
  
  // Compute time for full motion
  float dx = hw_state.coordinate_x_goal-hw_state.coordinate_x_start;
  float dy = hw_state.coordinate_y_goal-hw_state.coordinate_y_start;
  hw_state.motor_motion_time_since_start = 0;
  hw_state.motor_motion_time = sqrt(dx*dx+dy*dy)*60000000/hw_state.motor_speed_mm_per_min;

  // Set direction
  if(hw_state.motor_pos_right_delta < 0)
      digitalWrite(PIN_DIR_RIGHT, 1);
  else
      digitalWrite(PIN_DIR_RIGHT, 0);
      
  // Set direction
  if(hw_state.motor_pos_left_delta < 0)
      digitalWrite(PIN_DIR_LEFT, 1);
  else
      digitalWrite(PIN_DIR_LEFT, 0);
      
#ifdef DEBUG_PRINTS
  Serial.println("MotionTime: " + String(hw_state.motor_motion_time));
  Serial.println("dx: " + String((int)(dx)));
  Serial.println("dy: " + String((int)(dy)));
  Serial.println("DeltaX: " + String(hw_state.motor_pos_left_delta));
  Serial.println("DeltaY: " + String(hw_state.motor_pos_right_delta));
  Serial.println("length: " + String((int)(sqrt(dx*dx+dy*dy))));
  Serial.println("Speed: " + String(hw_state.motor_speed_mm_per_min));
  Serial.println("GoalL: " + String(hw_state.motor_pos_left_goal));
  Serial.println("GoalR: " + String(hw_state.motor_pos_right_goal));
  Serial.println("StartL: " + String(hw_state.motor_pos_left_start));
  Serial.println("StartR: " + String(hw_state.motor_pos_right_start));
#endif
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
  if(hw_state.motor_motion_time == 0)
    return;

  
  // Return if steppers have to wait
  //if (hw_state.timer0_us_since_last_step < hw_state.motor_us_per_step || hw_ctrl_final_pos_reached())
  //  return;

  //hw_state.timer0_us_since_last_step = 0;

  float t = (float)hw_state.motor_motion_time_since_start/hw_state.motor_motion_time;
  
  int32_t dL1 = hw_state.motor_pos_left_delta*t;
  int32_t stepsL = dL1 - hw_state.motor_pos_left_delta_last;
  hw_state.motor_pos_left_delta_last = dL1;
  
  int32_t dR1 = hw_state.motor_pos_right_delta*t;
  int32_t stepsR = dR1 - hw_state.motor_pos_right_delta_last;
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
      //hw_state.motor_pos_left_delta = 0;
      hw_state.motor_pos_right_start = hw_state.motor_pos_right_goal;
      //hw_state.motor_pos_right_delta = 0;
      //hw_state.motor_motion_time_since_start = 0;
      hw_state.motor_motion_time = 0;
  }

  //Serial.println("Moving dL: " + String(hw_state.motor_pos_left_goal - hw_state.motor_pos_left_curr));
  //Serial.println("Moving dR: " + String(hw_state.motor_pos_right_goal - hw_state.motor_pos_right_curr));
//  int8_t dl = 0;
//  int8_t dr = 0;
//
//  if (hw_state.motor_pos_left_goal < hw_state.motor_pos_left_curr) {
//    dl = -1;
//    digitalWrite(PIN_DIR_LEFT, 1);
//  }
//  else if ((hw_state.motor_pos_left_goal > hw_state.motor_pos_left_curr)) {
//    dl = 1;
//    digitalWrite(PIN_DIR_LEFT, 0);
//  }
//  if (hw_state.motor_pos_right_goal < hw_state.motor_pos_right_curr) {
//    dr = -1;
//    digitalWrite(PIN_DIR_RIGHT, 1);
//  }
//  else if ((hw_state.motor_pos_right_goal > hw_state.motor_pos_right_curr)) {
//    dr = 1;
//    digitalWrite(PIN_DIR_RIGHT, 0);
//  }
//
//  // Execute step if necessary
//  if (dl != 0) {
//    hw_state.motor_pos_left_curr += dl;
//    digitalWrite(PIN_STEP_LEFT, 1);
//    digitalWrite(PIN_STEP_LEFT, 0);
//  }
//  if (dr != 0) {
//    hw_state.motor_pos_right_curr += dr;
//    digitalWrite(PIN_STEP_RIGHT, 1);
//    digitalWrite(PIN_STEP_RIGHT, 0);
//  }
}

bool hw_ctrl_final_pos_reached() {
  return hw_state.motor_motion_time == 0;
}



#endif

