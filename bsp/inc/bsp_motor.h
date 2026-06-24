/**
 * @file    bsp_motor.h
 * @author  DinhTien (tien.ta.eswe@gmail.com)
 * @brief   Board Support Package (BSP) for DC Motor H-Bridge Driver.
 * @details Provides high-level abstraction interfaces to control a DC motor 
 * utilizing an H-Bridge circuit. Manages PWM duty cycle configuration 
 * for speed regulation and GPIO sequencing for direction switching and braking.
 * @version 1.0
 * @date    2026-06-23
 *
 * @copyright Copyright (c) 2026
 *
 */
#ifndef BSP_MOTOR_H_
#define BSP_MOTOR_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

/*******************************************************************************
 * API
 ******************************************************************************/

/**
 * @brief  Initializes the DC motor driver hardware peripherals.
 * @note   Configures direction control GPIO pins (IN1/IN2) as outputs and 
 * sets up the corresponding Timer channel for safe PWM generation.
 * @return None
 */
void BSP_Motor_Init(void);

/**
 * @brief  Updates the motor running direction and rotational speed.
 * @param  speed: Signed speed factor bounded from -10000 to 10000 
 * (representing -100.00% reverse to +100.00% forward velocity).
 * @return None
 */
void BSP_Motor_SetSpeed(int16_t speed);

/**
 * @brief  Executes a hard electronic brake sequence on the H-Bridge.
 * @note   Short-circuits both motor terminals to ground (or VCC depending on hardware layout) 
 * to ignite rapid back-EMF counter-torque deceleration.
 * @return None
 */
void BSP_Motor_Brake(void);

#ifdef __cplusplus
}
#endif

#endif /* BSP_MOTOR_H_ */