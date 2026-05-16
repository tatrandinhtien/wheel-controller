/**
 * @file bsp_servo.h
 * @brief BSP Servo API (stub)
 */

#ifndef BSP_SERVO_H_
#define BSP_SERVO_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

void BSP_Servo_Init(void);
void BSP_Servo_SetAngle(uint8_t angle);

#ifdef __cplusplus
}
#endif

#endif /* BSP_SERVO_H_ */
