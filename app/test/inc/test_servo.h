/**
 * @file    test_servo.h
 * @author  DinhTien (tien.ta.eswe@gmail.com)
 * @brief   Testing framework interface for the RC Servo motor module.
 * @version 1.0
 * @date    2026-06-24
 *
 * @copyright Copyright (c) 2026
 *
 */

#ifndef TEST_SERVO_H
#define TEST_SERVO_H

#ifdef __cplusplus
extern "C" {
#endif

/*******************************************************************************
 * API
 ******************************************************************************/

/**
 * @brief  Initializes components, spawns the RTOS test thread, and starts the kernel scheduler.
 * @note   This function contains an infinite blocking kernel execution loop and will not return.
 * @return None
 */
void test_servo_run(void);

#ifdef __cplusplus
}
#endif

#endif /* TEST_SERVO_H */