/**
 * @file    test_encoder.h
 * @author  DinhTien (tien.ta.eswe@gmail.com)
 * @brief   Testing framework interface for closed-loop motor velocity PID control.
 * @version 1.0
 * @date    2026-06-24
 *
 * @copyright Copyright (c) 2026
 *
 */

#ifndef TEST_ENCODER_H
#define TEST_ENCODER_H

#ifdef __cplusplus
extern "C" {
#endif

/*******************************************************************************
 * API
 ******************************************************************************/

/**
 * @brief  Initializes peripherals, links RTOS feedback loops, and fires the control tasks.
 * @note   Spawns independent threads for Encoder Feedback (10ms), PID Regulation (20ms), 
 * and Telemetry Plotting (50ms). This loop blocks execution permanently.
 * @return None
 */
void test_encoder_control_run(void);

#ifdef __cplusplus
}
#endif

#endif /* TEST_ENCODER_H */