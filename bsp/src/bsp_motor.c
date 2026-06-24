/**
 * @file    bsp_motor.c
 * @author  DinhTien (tien.ta.eswe@gmail.com)
 * @brief   Board Support Package (BSP) for DC Motor H-Bridge Driver implementation.
 * @version 1.0
 * @date    2026-05-30
 *
 * @copyright Copyright (c) 2026
 *
 */

#include "bsp_motor.h"
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

void BSP_Motor_Init(void) {
    /* Configure PWM pin as Alternate Function Push-Pull output */
    Driver_GPIO0.Setup(MOTOR_PWM_PIN, NULL);
    Driver_GPIO0.SetDirection(MOTOR_PWM_PIN, ARM_GPIO_AF_OUTPUT);
    Driver_GPIO0.SetOutputMode(MOTOR_PWM_PIN, ARM_AFIO_PUSH_PULL);

    /* Configure Bridge Direction control lines (IN1/IN2) as standard outputs */
    Driver_GPIO0.Setup(MOTOR_IN1, NULL);
    Driver_GPIO0.SetDirection(MOTOR_IN1, ARM_GPIO_OUTPUT);
    Driver_GPIO0.SetOutputMode(MOTOR_IN1, ARM_GPIO_PUSH_PULL);

    Driver_GPIO0.Setup(MOTOR_IN2, NULL);
    Driver_GPIO0.SetDirection(MOTOR_IN2, ARM_GPIO_OUTPUT);
    Driver_GPIO0.SetOutputMode(MOTOR_IN2, ARM_GPIO_PUSH_PULL);

    /* Short both motor terminals to ground for safety */
    Driver_GPIO0.SetOutput(MOTOR_IN1, LOW);
    Driver_GPIO0.SetOutput(MOTOR_IN2, LOW);

    /* Initialize Timer 1 Channel 1 for PWM carrier generation */
    Driver_TIM0.Setup(ARM_TIM_1, PWM_FREQ);
    Driver_TIM0.SetMode(ARM_TIM_1, ARM_CHANNEL_1);
    Driver_TIM0.SetDuty(ARM_TIM_1, ARM_CHANNEL_1, 0U);
    Driver_TIM0.Trigger(ARM_TIM_1, ARM_CHANNEL_1);
}

void BSP_Motor_SetSpeed(int16_t speed) {
    uint16_t duty_cycle = 0;

    /* Software Saturation: Enforce input constraints limits */
    if (speed > 130)  speed = 130;
    if (speed < -130) speed = -130;

    if (speed > 0) {
        /* Forward Operation Mode */
        Driver_GPIO0.SetOutput(MOTOR_IN1, HIGH);
        Driver_GPIO0.SetOutput(MOTOR_IN2, LOW);
        
        /* Map speed scale onto standard duty range (0-10000) */
        duty_cycle = (uint16_t)(((uint32_t)speed * 10000U) / 130U); 
    } 
    else if (speed < 0) {
        /* Reverse Operation Mode */
        Driver_GPIO0.SetOutput(MOTOR_IN1, LOW);
        Driver_GPIO0.SetOutput(MOTOR_IN2, HIGH);
        
        /* Negate signed input to obtain absolute scale factor for duty calculation */
        duty_cycle = (uint16_t)(((uint32_t)(-speed) * 10000U) / 130U);
    } 
    else {
        /* Neutral/Idle State */
        Driver_GPIO0.SetOutput(MOTOR_IN1, LOW);
        Driver_GPIO0.SetOutput(MOTOR_IN2, LOW);
        duty_cycle = 0;
    }

    /* Write scaled value to active compare register */
    Driver_TIM0.SetDuty(ARM_TIM_1, ARM_CHANNEL_1, duty_cycle);
}

void BSP_Motor_Brake(void) {
    /* Ground both terminal inputs to short circuit counter-electromotive coils */
    Driver_GPIO0.SetOutput(MOTOR_IN1, LOW);
    Driver_GPIO0.SetOutput(MOTOR_IN2, LOW);
}