/**
 * @file    bsp_hbridge.c
 * @author  DinhTien (tien.ta.eswe@gmail.com)
 * @brief   Board Support Package (BSP) for DC Motor H-Bridge Driver implementation.
 * @details Manages a DC motor configuration using a hardware H-Bridge structure. 
 * Maps GPIO pins for directional control steering loops and drives Timer 1 Channel 1 
 * (TIM1_CH1) to generate high-frequency PWM for velocity control. Includes mathematical 
 * mapping scaling factors and functional safety braking.
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

extern ARM_DRIVER_TIM_PWM Driver_TIM0; /**< Linked HAL driver instance for Timer PWM configurations. */
extern ARM_DRIVER_GPIO    Driver_GPIO0;/**< Linked HAL driver instance for GPIO pin configuration. */

/*******************************************************************************
 * Code
 ******************************************************************************/

/**
 * @brief  Initializes the physical H-Bridge GPIO logic control lines and PWM Timer counter.
 * @note   Maps MOTOR_PWM_PIN to Alternate Function mode, sets direction pins (IN1/IN2) as 
 * outputs initialized to logic LOW, and activates TIM1_CH1 at the custom frequency.
 * @return None
 */
void BSP_Motor_Init(void) {

    /* Configure PWM line pin as Alternate Function Push-Pull output mapped to the Timer peripheral */
    Driver_GPIO0.Setup(MOTOR_PWM_PIN, NULL);
    Driver_GPIO0.SetDirection(MOTOR_PWM_PIN, ARM_GPIO_AF_OUTPUT);
    Driver_GPIO0.SetOutputMode(MOTOR_PWM_PIN, ARM_AFIO_PUSH_PULL);

    /* Configure Bridge Direction control line 1 (IN1) as a standard general-purpose output */
    Driver_GPIO0.Setup(MOTOR_IN1, NULL);
    Driver_GPIO0.SetDirection(MOTOR_IN1, ARM_GPIO_OUTPUT);
    Driver_GPIO0.SetOutputMode(MOTOR_IN1, ARM_GPIO_PUSH_PULL);

    /* Configure Bridge Direction control line 2 (IN2) as a standard general-purpose output */
    Driver_GPIO0.Setup(MOTOR_IN2, NULL);
    Driver_GPIO0.SetDirection(MOTOR_IN2, ARM_GPIO_OUTPUT);
    Driver_GPIO0.SetOutputMode(MOTOR_IN2, ARM_GPIO_PUSH_PULL);

    /* Enforce a safe default baseline configuration: Short both motor terminals to ground to prevent drifting */
    Driver_GPIO0.SetOutput(MOTOR_IN1, LOW);
    Driver_GPIO0.SetOutput(MOTOR_IN2, LOW);

    /* Initialize Advanced Timer 1 Channel 1 to fire carrier frequency waves configured inside bsp_config.h */
    Driver_TIM0.Setup(ARM_TIM_1, PWM_FREQ);
    Driver_TIM0.SetMode(ARM_TIM_1, ARM_CHANNEL_1);
    Driver_TIM0.SetDuty(ARM_TIM_1, ARM_CHANNEL_1, 0U);
    Driver_TIM0.Trigger(ARM_TIM_1, ARM_CHANNEL_1);
}

/**
 * @brief  Updates the direction sequence logic states and computes scaled target PWM duties.
 * @note   Handles software saturation constraints between -130 and +130 absolute input values.
 * Performs safe integer conversion math to map values onto a 0-10000 range.
 * @param  speed: Signed acceleration/velocity directive value (Positive = Forward, Negative = Reverse).
 * @return None
 */
void BSP_Motor_SetSpeed(int16_t speed) {
    uint16_t duty_cycle = 0;

    /* Software Saturation: Bound input parameters within maximum physical motor driver tolerances */
    if (speed > 130)  speed = 130;
    if (speed < -130) speed = -130;

    if (speed > 0) {
        /* Forward Operation Mode sequence logic layout */
        Driver_GPIO0.SetOutput(MOTOR_IN1, HIGH);
        Driver_GPIO0.SetOutput(MOTOR_IN2, LOW);
        
        /* Map speed scale factor onto proportional resolution duty range: duty = (speed * 10000) / 130 */
        duty_cycle = (uint16_t)(((uint32_t)speed * 10000U) / 130U); 
    } 
    else if (speed < 0) {
        /* Reverse Operation Mode sequence logic layout */
        Driver_GPIO0.SetOutput(MOTOR_IN1, LOW);
        Driver_GPIO0.SetOutput(MOTOR_IN2, HIGH);
        
        /* Negate the negative signed scale input to obtain absolute factor values for mapping calculations */
        duty_cycle = (uint16_t)(((uint32_t)(-speed) * 10000U) / 130U);
    } 
    else {
        /* Neutral/Idle State: Drop bridge operational control lines to default low voltage states */
        Driver_GPIO0.SetOutput(MOTOR_IN1, LOW);
        Driver_GPIO0.SetOutput(MOTOR_IN2, LOW);
        duty_cycle = 0;
    }

    /* Commit the newly calculated proportional duty cycle down onto the active Timer registries */
    Driver_TIM0.SetDuty(ARM_TIM_1, ARM_CHANNEL_1, duty_cycle);
}

/**
 * @brief  Forces a hard electronic emergency deceleration loop onto the H-Bridge.
 * @note   Drives both bridge input terminals (IN1/IN2) to logic LOW states to dissipate residual kinetic energy.
 * @return None
 */
void BSP_Motor_Brake(void) {
    /* Ground both terminal inputs to short circuit counter-electromotive coils forces */
    Driver_GPIO0.SetOutput(MOTOR_IN1, LOW);
    Driver_GPIO0.SetOutput(MOTOR_IN2, LOW);
}