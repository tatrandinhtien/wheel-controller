/**
 * @file    bsp_servo.c
 * @author  DinhTien (tien.ta.eswe@gmail.com)
 * @brief   Board Support Package (BSP) for RC Servo Motor implementation.
 * @version 1.0
 * @date    2026-05-30
 *
 * @copyright Copyright (c) 2026
 *
 */

#include "bsp_servo.h"
#include "bsp_config.h"
#include "Driver_Timer.h"
#include "Driver_GPIO.h"
#include <stddef.h>

/*******************************************************************************
 * Variables
 ******************************************************************************/

extern ARM_DRIVER_TIM_PWM Driver_TIM0; 
extern ARM_DRIVER_GPIO    Driver_GPIO0;

/*******************************************************************************
 * Code
 ******************************************************************************/

void BSP_Servo_Init(void) {
    /* Route IO pins to Alternate Function Push-Pull mode */
    Driver_GPIO0.Setup(SERVO_PWM_PIN, NULL);
    Driver_GPIO0.SetDirection(SERVO_PWM_PIN, ARM_GPIO_AF_OUTPUT);
    Driver_GPIO0.SetOutputMode(SERVO_PWM_PIN, ARM_AFIO_PUSH_PULL);

    /* Setup Timer 4 Channel 1 to fire 50Hz carrier frequency */
    Driver_TIM0.Setup(ARM_TIM_4, SERVO_FREQ);
    Driver_TIM0.SetMode(ARM_TIM_4, ARM_CHANNEL_1);
    Driver_TIM0.Trigger(ARM_TIM_4, ARM_CHANNEL_1);

    /* Force default center position */
    BSP_Servo_SetAngle(0);
}

void BSP_Servo_SetAngle(int16_t final_angle) {
    /* Software Saturation: Enforce mechanical bounds limits (-90.0 to +90.0 degrees) */
    if (final_angle > 900)  final_angle = 900;
    if (final_angle < -900) final_angle = -900;

    /* Transform angular inputs into absolute pulse durations (1000us to 2000us) */
    uint16_t pulse_us = 1500 + ((int32_t)final_angle * 1000) / 900;
    
    /* Calculate registration scale duty value relative to Timer setup (pulse_us / 2) */
    uint16_t duty_scaled = pulse_us / 2;

    /* Write scaled value to active compare register */
    Driver_TIM0.SetDuty(ARM_TIM_4, ARM_CHANNEL_1, duty_scaled);
}