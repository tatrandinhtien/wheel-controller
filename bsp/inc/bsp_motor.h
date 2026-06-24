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
 * @details Handles software saturation and maps the speed value onto the 
 * internal driver's proportional duty cycle range (0 to 10000).
 * @param  speed: Signed speed directive factor bounded from -130 (max reverse) 
 * to +130 (max forward velocity). A value of 0 triggers neutral idle.
 * @return None
 */
void BSP_Motor_SetSpeed(int16_t speed);

/**
 * @brief  Executes a hard electronic brake sequence on the H-Bridge.
 * @note   Short-circuits both motor terminals to ground to dissipate residual 
 * kinetic energy via counter-electromotive forces.
 * @return None
 */
void BSP_Motor_Brake(void);

#ifdef __cplusplus
}
#endif

#endif /* BSP_MOTOR_H_ */