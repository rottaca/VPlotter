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
#define CORD_LENGTH_PER_REVOLUTION M_PI*5.0f
// Full steps necessary for a single rotation
#define STEPPER_STEPS_PER_REVOLUTION 200
// Speed of timer0, used for servo positioning and stepper control
#define TIMER1_US_PER_INTERRUPT 100
#define SPEED_MULTIPLIER 2

// Pin layout
#define PIN_DIR_LEFT 6
#define PIN_DIR_RIGHT 2
#define PIN_STEP_LEFT 7
#define PIN_STEP_RIGHT 3
#define PIN_SERVO 9
#define PIN_LED 13

// Valid drawing area, Movements are clamped to this area
#define X_MIN 10
#define X_MAX 600
#define Y_MIN 10
#define Y_MAX 1100

// Defines the home position
#define HOME_POS_X 300
#define HOME_POS_Y 300

// Servo setup
#define SERVO_US_FULL_PHASE 20000   // 20ms
#define SERVO_US_POS_UP 1800        // 1.8ms
#define SERVO_US_POS_DOWN 1000      // 1ms
#define SERVO_MOVE_DELAY 200000      // Time for servo to reach position 20 ms

// Macro to convert cord length into steps
#define LENGTH_TO_STEPS(l) ((l)/(CORD_LENGTH_PER_REVOLUTION) * STEPPER_STEPS_PER_REVOLUTION * MOTOR_MICRO_STEPPING )
#define STEPS_TO_LENGTH(s) ((s) *CORD_LENGTH_PER_REVOLUTION /(STEPPER_STEPS_PER_REVOLUTION * MOTOR_MICRO_STEPPING))
#define CLAMP(V,MIN,MAX) ((V)<(MIN)?(MIN):((V)>(MAX)?(MAX):(V)))

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
  int32_t err,es,el;
  int32_t s[STP_CNT],dp[STP_CNT],dd[STP_CNT];
  int32_t dSteps[STP_CNT];
  bool moveing;

  // motor position
  int32_t motor_pos[STP_CNT];
  int32_t motor_pos_target[STP_CNT];

  uint32_t servo_move_delay;
  // Time since last servo signal
  uint32_t servo_signal_length_us;
  // Servo up-time in microseconds -> Defines servo position
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
    hw_state.dd[i] = 0;
    hw_state.dp[i] = 0;
    hw_state.dSteps[i] = 0;
    hw_state.motor_pos[i] = 0;
    hw_state.motor_pos_target[i] = 0;
    hw_state.s[i] = 0;
  }

  hw_state.base = 0;
  hw_state.el = 0;
  hw_state.err = 0;
  hw_state.es = 0;
  hw_state.moveing = false;
  hw_state.servo_move_delay = 0;
  hw_state.us_since_servo_period = 0;
  hw_state.state = START;
  hw_state.servo_signal_length_us = SERVO_US_POS_UP;

  // Initialize hardware
  pinMode(PIN_DIR_LEFT, OUTPUT);
  pinMode(PIN_DIR_RIGHT, OUTPUT);
  pinMode(PIN_STEP_LEFT, OUTPUT);
  pinMode(PIN_STEP_RIGHT, OUTPUT);
  pinMode(PIN_SERVO, OUTPUT);
  pinMode(PIN_LED,OUTPUT);

  // Lift the pen
  hw_ctrl_set_drawing(false);

  // Start timer for motor control
  Timer1.initialize(TIMER1_US_PER_INTERRUPT);
  Timer1.attachInterrupt(hw_ctrl_timer_callback);

  // Init servo timer by hand
  // F_CPU = 16000000Hz (16 MHz)
  // Timer 2: 8-Bit -> 256
  // Prescaler 8 -> 16000000Hz/8 =2000000Hz
  // 1000/2000000Hz = 0,0005ms per Tick
  // 0,0005ms*256 = 0,128 ms per Overflow
  TCCR2A = 0;
  // Prescaler 8
  TCCR2B = (1 << CS21);
  // Enable overflow interrupt
  TIMSK2 = (1 << TOIE2);

}

bool hw_ctrl_set_drawing(bool drawing)
{
  if(hw_state.state != IDLE)
    return false;

  hw_state.servo_signal_length_us = drawing?SERVO_US_POS_DOWN:SERVO_US_POS_UP;
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
  float y = sqrt(lengthR * lengthR - (hw_state.base - x) * (hw_state.base - x));

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
  digitalWrite(PIN_DIR_RIGHT,(hw_state.s[STP_RIGHT])>0?1:0);
  digitalWrite(PIN_DIR_LEFT,(hw_state.s[STP_LEFT])>0?1:0);

  // Start motion
  hw_state.state = MOVING;

}

void hw_ctrl_timer_callback() {
  switch (hw_state.state) {
    case START:
    case IDLE:
      break;
    case MOVING:
    {
        // Motion finished ?
        if(hw_state.motor_pos[STP_LEFT] == hw_state.motor_pos_target[STP_LEFT] &&
          hw_state.motor_pos[STP_RIGHT] == hw_state.motor_pos_target[STP_RIGHT])
        {
            hw_state.state = IDLE;
            break;
        }
        // Optimized and faster bresenham line algorithm
        // Source:http://members.chello.at/easyfilter/bresenham.html
        int32_t e2 = 2*hw_state.err;
        if(e2 >= -hw_state.dSteps[STP_RIGHT]){
          hw_state.err -= hw_state.dSteps[STP_RIGHT];
          digitalWrite(PIN_STEP_LEFT, 1);
          digitalWrite(PIN_STEP_LEFT, 0);
          hw_state.motor_pos[STP_LEFT] += hw_state.s[STP_LEFT];
        }
        if(e2<= hw_state.dSteps[STP_LEFT]){
          hw_state.err += hw_state.dSteps[STP_LEFT];
          digitalWrite(PIN_STEP_RIGHT, 1);
          digitalWrite(PIN_STEP_RIGHT, 0);
          hw_state.motor_pos[STP_RIGHT] += hw_state.s[STP_RIGHT];
        }
        /**
        // Bresenham line algorithm
        hw_state.err -= 2*hw_state.es;
        if(hw_state.err < 0){
          hw_state.err += 2*hw_state.el;
          digitalWrite(PIN_STEP_LEFT, 1);
          digitalWrite(PIN_STEP_LEFT, 0);
          hw_state.motor_pos[STP_LEFT] += hw_state.dd[STP_LEFT];

          digitalWrite(PIN_STEP_RIGHT, 1);
          digitalWrite(PIN_STEP_RIGHT, 0);
          hw_state.motor_pos[STP_RIGHT] += hw_state.dd[STP_RIGHT];

        }else{
          if(hw_state.dp[STP_LEFT] != 0){
            digitalWrite(PIN_STEP_LEFT, 1);
            digitalWrite(PIN_STEP_LEFT, 0);
            hw_state.motor_pos[STP_LEFT] += hw_state.dp[STP_LEFT];
          }
          if(hw_state.dp[STP_RIGHT] != 0){
            digitalWrite(PIN_STEP_RIGHT, 1);
            digitalWrite(PIN_STEP_RIGHT, 0);
            hw_state.motor_pos[STP_RIGHT] += hw_state.dp[STP_RIGHT];
          }
        }
        */
    }
    break;
  case WAIT_SERVO:
    hw_state.servo_move_delay += TIMER1_US_PER_INTERRUPT;
    if(hw_state.servo_move_delay>= SERVO_MOVE_DELAY){
      hw_state.state = IDLE;
    }
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

void hw_ctrl_convert_length_to_point(float L, float R, float* x, float* y)
{
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

// ISR for servo positioning
ISR(TIMER2_OVF_vect){
    // 128 us per timer overflow
    hw_state.us_since_servo_period += 128;

    if (hw_state.us_since_servo_period >= SERVO_US_FULL_PHASE) {
      hw_state.us_since_servo_period = 0;
      digitalWrite(PIN_SERVO, 1);
    } else if (hw_state.us_since_servo_period > hw_state.servo_signal_length_us) {
      digitalWrite(PIN_SERVO, 0);
    }
}

#endif
