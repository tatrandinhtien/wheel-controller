/**
 * @file    bsp_servo.h
 * @author  DinhTien (tien.ta.eswe@gmail.com)
 * @brief   Board Support Package (BSP) for RC Servo Motor control.
 * @details Provides hardware abstraction layer APIs to drive an RC Servo motor 
 * using a dedicated Timer PWM channel. Manages angle-to-pulse-width 
 * conversion mapping based on standard 50Hz RC servo protocol.
 * @version 1.0
 * @date    2026-06-23
 *
 * @copyright Copyright (c) 2026
 *
 */

#ifndef BSP_SERVO_H
#define BSP_SERVO_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

/*******************************************************************************
 * API
 ******************************************************************************/

/**
 * @brief  Initializes the specific Timer PWM channel assigned to the Servo motor.
 * @note   Configures the underlying timer base to generate a strict 50Hz carrier frequency 
 * (20ms total period layout) required for standard RC servo signaling.
 * @return None
 */
void BSP_Servo_Init(void);

/**
 * @brief  Updates the physical position of the Servo horn to the target angle.
 * @details Converts the angular parameter into a proportional pulse duration 
 * bounded strictly between 1ms (1000us) and 2ms (2000us) thresholds.
 * @param  final_angle: Target steering position angle in tenths of a degree.
 * Valid dynamic range is from -900 (-90.0°) to +900 (+90.0°).
 * @return None
 */
void BSP_Servo_SetAngle(int16_t final_angle);

#ifdef __cplusplus
}
#endif

#endif /* BSP_SERVO_H */