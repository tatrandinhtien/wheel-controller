// ================= HEADER & CONFIG =================
#include <Arduino.h>             // Arduino core types & macros
#include "timer.h"              // Timer & interrupt setup
#include "Send_Recieve_data.h"  // Data communication structs & functions

#define rs485_rate 500000        // RS485 baudrate
#define Max485_drive 4           // RS485 driver pin
  
// ================= CONTROL VARIABLES =================
// Motor & Servo
float Measure_speed = 0;         // Measured speed
float Motor_speed = 0;           // Target speed
int8_t Motor_directionF;         // Forward direction flag
int8_t Motor_directionB;         // Backward direction flag
float FrontLeft_Angle = 0;       // Servo angle
float Set_Angle = 0;             // Target angle

// Speed calculation
float counter = 0;
float rpm = 0;
float rpmA = 0;
float rpmB = 0;
float Target_speed;
float Set_motor_speed;
float global_error;
int8_t motor_state;
uint16_t x;

// ================= PID CONTROL =================
float deltaT = 0.02;             // Sample time (s)
float error = 0;
float de_dt = 0;
float e_prev = 0;
float e_integral = 0;
float Output;
float Kp = 4;                    // Proportional gain
float Ki = 3;                    // Integral gain
float Kd = 0.001;                // Derivative gain

// PID_2 (for advanced control)
float error_2 = 0;               // e(t-2)
float error_1 = 0;               // e(t-1)
float error_0 = 0;               // e(t)
float output = 0;                // Actuator output

// ================= ENCODER & SPEED MEASUREMENT =================
bool Flag_50Hz = 0;
bool Flag_16kHz = 0;
uint8_t count_50Hz;
uint8_t count_16kHz;
int In0_chanelA_read;
int In1_chanelB_read;
float T_timer3 = 0.0000005;      // Timer3 tick (8/16 MHz)
uint16_t periodA;
uint16_t periodB;
uint16_t ICR1_lastA;
uint16_t ICR1_firstA;
uint16_t ICR1_lastB;
uint16_t ICR1_firstB;
uint16_t lastA;
uint16_t firstA;
uint16_t lastB;
uint16_t firstB;
int16_t usable_counterA = 0;
int16_t usable_counterB = 0;
int16_t counterA = 0;
int16_t counterB = 0;

// ================= CONSTANTS =================
float K = 255863.5394;           // Encoder constant



// ================= SETUP =================
void setup() {
  // --- Pin configuration ---
  pinMode(A0, INPUT_PULLUP); // DIP switch
  pinMode(A1, INPUT_PULLUP);
  pinMode(A2, INPUT_PULLUP);
  pinMode(A3, INPUT_PULLUP);

  pinMode(9, OUTPUT);        // Motor PWM
  pinMode(2, INPUT_PULLUP);  // Encoder channel A
  pinMode(3, INPUT_PULLUP);  // Encoder channel B
  pinMode(5, OUTPUT);        // Servo PWM
  pinMode(6, OUTPUT);        // Motor in1
  pinMode(7, OUTPUT);        // Motor in2
  pinMode(Max485_drive, OUTPUT); // RS485 driver
  digitalWrite(Max485_drive, LOW);

  // --- Interrupts for encoder ---
  attachInterrupt(digitalPinToInterrupt(2), Count_pulses_A, CHANGE);
  attachInterrupt(digitalPinToInterrupt(3), Count_pulses_B, CHANGE);

  // --- Serial communication ---
  Serial1.begin(rs485_rate); // RS485
  Serial.begin(115200);      // Debug

  // --- Slave ID setup ---
  slave_IP = PINF & B11110000; // 8 bit
  Modular_ID = slave_IP / 16;  >> 4
  Sending_order = ((Modular_ID == 2) ? 11 : (Modular_ID == 3) ? 12 : (Modular_ID == 4) ? 13 : (Modular_ID == 5) ? 14 : (Modular_ID == 6) ? 15 : 0);
  send_data.Footer[2] = Modular_ID;
  send_data.Header[2] = Sending_order;

  // --- Timer initialization ---
  Timer0();
  Timer1();
  Timer3();
}


// ================= MAIN LOOP =================
void loop() {
  // --- 50Hz control cycle ---
  if (Flag_50Hz) {
    Flag_50Hz = 0;
    // Motor & servo control
    Motor_state();
    Setservo_angle(Set_Angle);
    // Speed measurement & PID
    Cal_speed();
    PID_controller(abs(Motor_speed));
    Setmotor_speed(Set_motor_speed);
  }

  // --- Receive new control data from master ---
  if (FL_flag == 1) {
    FL_flag = 0;
    Set_Angle = (float)recieve_data.FL_Angle / 10.0;
    Motor_speed = (float)recieve_data.FL_Speed / 10.0;
  }
  if (FR_flag == 1) {
    FR_flag = 0;
    Set_Angle = (float)recieve_data.FR_Angle / 10.0;
    Motor_speed = (float)recieve_data.FR_Speed / 10.0;
  }
  if (RL_flag == 1) {
    RL_flag = 0;
    Set_Angle = (float)recieve_data.RL_Angle / 10.0;
    Motor_speed = (float)recieve_data.RL_Speed / 10.0;
  }
  if (RR_flag == 1) {
    RR_flag = 0;
    Set_Angle = (float)recieve_data.RR_Angle / 10.0;
    Motor_speed = (float)recieve_data.RR_Speed / 10.0;
  }

  // --- Prepare data to send back to master ---
  if (send_prepare_flag == 1) {
    send_prepare_flag = 0;
    send_data.confirm_angle = int16_t(Set_Angle * 10);
    send_data.confirm_speed = int16_t(Motor_speed * 10);
    send_data.measure_speed = int16_t(Measure_speed * 10);
    send_data.angle_PWM_signal = int16_t(OCR3A * 10);
    send_data.speed_PWM_signal = int16_t(OCR1A * 10);
    send_flag = 1;
  }
}

// ================= INTERRUPT SERVICE ROUTINES =================
ISR(TIMER0_COMPA_vect) {
  // Timer0 compare match: handle RS485 communication
  Sending_data();
}

ISR(TIMER1_CAPT_vect) {
  // Timer1 input capture: (reserved for future use)
  uint16_t input_capture = ICR1;
  // counter++;
}

ISR(TIMER1_OVF_vect) {
  // Timer1 overflow: reset timer
  TCNT1 = 24;
}

ISR(TIMER3_OVF_vect) {
  // Timer3 overflow: 50Hz control cycle
  Flag_50Hz = 1;
  lastA = ICR1_lastA;
  firstA = ICR1_firstA;
  lastB = ICR1_lastB;
  firstB = ICR1_firstB;
  usable_counterA = counterA;
  usable_counterB = counterB;
  counterA = 0;
  counterB = 0;
}

// ================= CONTROL FUNCTIONS =================

// Motor direction control
void Motor_state() {
  if (Motor_speed > 0) {
    Forward();
  } else if (Motor_speed < 0) {
    Backward();
  } else if (Motor_speed == 0) {
    Stop();
  }
}

// Set servo angle (PWM mapping)
void Setservo_angle(int16_t Servo_angle) {
  OCR3A = mapFloat(Servo_angle, -450, 450, 1600, 3000);
}

// Set motor speed (PWM mapping)
void Setmotor_speed(uint16_t Motor_speed) {
  OCR1A = mapFloat(Motor_speed, 0, 400, 24, 820); // 16.8/1023*12 + voltage drop
}

// Calculate speed from encoder
void Cal_speed() {
  periodA = ((lastA > firstA) ? (lastA - firstA) : (40000 - firstA + lastA)); 
  rpmA = (float)(((abs(usable_counterA) - 1) * K) / (periodA));
  periodB = ((lastB > firstB) ? (lastB - firstB) : (40000 - firstB + lastB));
  rpmB = (float)(((abs(usable_counterB) - 1) * K) / (periodB));
  rpm = (rpmA + rpmB) / 2.0;
  Measure_speed = rpm;
  if (Measure_speed >= 400) Measure_speed = 400;
  else if (Measure_speed <= 20) Measure_speed = 0;
}

// Encoder pulse counting (interrupt)
void Count_pulses_A() {
  if (counterA)
    ICR1_lastA = TCNT3;
  else
    ICR1_firstA = TCNT3;
  In0_chanelA_read = PIND & B00000011;
  counterA += ((In0_chanelA_read != 0) && (In0_chanelA_read != 3) ? (-1) : (+1));
}

void Count_pulses_B() {
  if (counterB)
    ICR1_lastB = TCNT3;
  else
    ICR1_firstB = TCNT3;
  In1_chanelB_read = PIND & B00000011;
  counterB += ((In1_chanelB_read != 0) && (In1_chanelB_read != 3) ? (-1) : (+1));
}

// PID controller (classic)
void PID_controller(int Tar_speed) {
  error = (Tar_speed - Measure_speed);
  e_integral += error * deltaT;
  e_integral = (e_integral > 400) ? 400 : (e_integral < -400) ? -400 : e_integral;
  de_dt = (error - e_prev) / deltaT;

  Output = Kp * error + Ki * e_integral + Kd * de_dt;
  Output = (Output > 400) ? 400 : (Output < 20) ? 0 : Output;
  Set_motor_speed = Output;
  global_error = error;
  e_prev = error;
}

// PID controller (advanced)
void PID_controller_2(int setpoint) {
  float A0 = Kp + Ki * deltaT + Kd / deltaT;
  float A1 = -Kp - 2 * Kd / deltaT;
  float A2 = Kd / deltaT;

  error_2 = error_1;
  error_1 = error_0;
  error_0 = (setpoint - Measure_speed);
  Output = Output + A0 * error_0 + A1 * error_1 + A2 * error_2;
  Output = (Output > 400) ? 400 : (Output < 20) ? 0 : Output;
  Set_motor_speed = Output;
}

// Motor direction
void Forward() {
  PORTE |= (1 << PE6);
  PORTD &= ~(1 << PD7);
}
void Backward() {
  PORTD |= (1 << PD7);
  PORTE &= ~(1 << PE6);
}
void Stop() {
  PORTE &= ~(1 << PE6);
  PORTD &= ~(1 << PD7);
}

// Utility: map float value from one range to another
float mapFloat(float x, float x_min, float x_max, float y_min, float y_max) {
  float y;
  y = y_min + (x - x_min) / (x_max - x_min) * (y_max - y_min);
  return y;
}
