/**
 * @file    test_encoder.c
 * @author  DinhTien (tien.ta.eswe@gmail.com)
 * @brief   Closed-loop motor velocity PID control regulation implementation under RTOS.
 * @version 1.0
 * @date    2026-06-23
 * * @copyright Copyright (c) 2026
 * */
#include <stdio.h>
#include <string.h>

#include "cmsis_os2.h"
#include "test_encoder.h"
#include "bsp_encoder.h"
#include "bsp_motor.h"
#include "bsp_log.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/

#define ENCODER_PPR       11.0f  
#define ENCODER_MODE_X4   4.0f   
#define GEAR_RATIO        45.0f  
#define TOTAL_PPR         (ENCODER_PPR * ENCODER_MODE_X4 * GEAR_RATIO) 

#define TS_ENCODER_MS     10U    
#define TS_PID_MS         20U    
#define TS_PLOTTER_MS     50U    
#define TS_PID_SEC        0.02f
#define TS_ENCODER_SEC    0.01f  

typedef struct {
    float kp;
    float ki;
    float kd;
    float integrator;
    float prev_error;
    float out_max;
    float out_min;
} PID_Controller_t;

/*******************************************************************************
 * Variables
 ******************************************************************************/

volatile int16_t target_rpm = 70;      
volatile int16_t actual_rpm = 0;       
volatile int16_t current_pwm_out = 0;  
static volatile float current_wheel_rpm = 0.0f;

static PID_Controller_t pid_speed = {
    .kp = 0.15f,
    .ki = 0.50f,
    .kd = 0.000f,
    .integrator = 0.0f,
    .prev_error = 0.0f,
    .out_max = 99.0f,
    .out_min = -99.0f
};

/*******************************************************************************
 * Prototypes
 ******************************************************************************/

static float PID_Compute(PID_Controller_t *pid, float target, float actual, float dt);
static void vEncoderFeedbackTask(void *argument);
static void vMotorPIDControlTask(void *argument);
static void vSerialPlotterTask(void *argument);

/*******************************************************************************
 * Code
 ******************************************************************************/

/**
 * @brief  Computes the positional PID correction factor with anti-windup clamping.
 * @param  pid: Target controller tracking context instance structure pointer.
 * @param  target: Desired setpoint baseline reference value.
 * @param  actual: Current process velocity feedback metrics.
 * @param  dt: Delta sampling timeframe interval period sequence.
 * @return float: Bound saturated control effort output command.
 */
static float PID_Compute(PID_Controller_t *pid, float target, float actual, float dt) {
    float error = target - actual;
    
    float p_term = pid->kp * error;
    
    /* Accumulate fractional integration step values */
    pid->integrator += error * dt;
    float i_term = pid->ki * pid->integrator;
    
    /* Integral Anti-windup clamping logic layout */
    if (i_term > pid->out_max) { i_term = pid->out_max; pid->integrator -= error * dt; }
    else if (i_term < pid->out_min) { i_term = pid->out_min; pid->integrator -= error * dt; }
    
    float d_term = pid->kd * (error - pid->prev_error) / dt;
    pid->prev_error = error;
    
    float output = p_term + i_term + d_term;
    
    /* Total output saturation check against physical limitations */
    if (output > pid->out_max) output = pid->out_max;
    else if (output < pid->out_min) output = pid->out_min;
    
    return output;
}

/**
 * @brief  High-frequency feedback tracking thread capturing encoder ticks.
 * @param  argument: Unused protocol pointer reference matching CMSIS prototypes.
 * @return None
 */
static void vEncoderFeedbackTask(void *argument) {
    (void)argument;
    uint32_t tick_start = osKernelGetTickCount();
    const uint32_t tick_freq = (TS_ENCODER_MS * osKernelGetTickFreq()) / 1000U;

    while(1) {
        /* Capture fractional signed difference ticks straight from hardware registers */
        int16_t delta_ticks = BSP_Encoder_GetDelta(); 
        
        /* Transform pulse ticks count linearly into physical wheel rotational RPM speeds */
        current_wheel_rpm = ((float)delta_ticks * 60.0f) / (TOTAL_PPR * TS_ENCODER_SEC);
        actual_rpm = (int16_t)current_wheel_rpm;

        tick_start += tick_freq;
        osDelayUntil(tick_start);
    }
}

/**
 * @brief  Time-critical controller task running the closed-loop velocity calculation.
 * @param  argument: Unused protocol pointer reference matching CMSIS prototypes.
 * @return None
 */
static void vMotorPIDControlTask(void *argument) {
    (void)argument;
    uint32_t tick_start = osKernelGetTickCount();
    const uint32_t tick_freq = (TS_PID_MS * osKernelGetTickFreq()) / 1000U;

    while(1) {
        /* Compute current control loop updates through the speed PID profile block */
        float pwm_output = PID_Compute(&pid_speed, (float)target_rpm, current_wheel_rpm, TS_PID_SEC);
        current_pwm_out = (int16_t)pwm_output;
        
        /* Write scaled acceleration output factor straight down to the motor bridge */
        BSP_Motor_SetSpeed((int32_t)current_pwm_out);

        tick_start += tick_freq;
        osDelayUntil(tick_start);
    }
}

/**
 * @brief  Telemetry diagnostic thread streaming dynamic real-time plotting data.
 * @param  argument: Unused protocol pointer reference matching CMSIS prototypes.
 * @return None
 */
static void vSerialPlotterTask(void *argument) {
    (void)argument;
    uint32_t tick_start = osKernelGetTickCount();
    const uint32_t tick_freq = (TS_PLOTTER_MS * osKernelGetTickFreq()) / 1000U;

    while(1) {
        /* Stream Telemetry data compatible with standard Serial Plotter graph tools */
        bsp_log_printf(">Target:%d\n", target_rpm);
        bsp_log_printf(">Actual:%d\n", actual_rpm);

        tick_start += tick_freq;
        osDelayUntil(tick_start);
    }
}

void test_encoder_control_run(void) {
    bsp_log_init();
    BSP_Encoder_Init();
    BSP_Motor_Init();

    setvbuf(stdout, NULL, _IONBF, 0);
    osKernelInitialize();

    /* Spawns the concurrent deterministic multi-rate loop contexts */
    const osThreadAttr_t encoder_attr = { .name = "Encoder", .priority = osPriorityNormal, .stack_size = 1024 };
    osThreadNew(vEncoderFeedbackTask, NULL, &encoder_attr);

    const osThreadAttr_t pid_attr = { .name = "PID", .priority = osPriorityNormal1, .stack_size = 2048 };
    osThreadNew(vMotorPIDControlTask, NULL, &pid_attr);

    const osThreadAttr_t plot_attr = { .name = "Plotter", .priority = osPriorityBelowNormal, .stack_size = 1024 };
    osThreadNew(vSerialPlotterTask, NULL, &plot_attr);

    osKernelStart();
    while (1) {}
}