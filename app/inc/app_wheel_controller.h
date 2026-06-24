/**
 * @file    app_wheel_controller.h
 * @author  DinhTien (tien.ta.eswe@gmail.com)
 * @brief   Application layer for distributed smart wheel node controller.
 * @details Manages independent distributed wheel nodes by orchestrating local 
 * sensors and actuators. Interfaces with the CAN bus to process coordinated 
 * steering/drive directives from a master ECU and returns localized odometer feedback.
 * @version 1.0
 * @date    2026-06-23
 *
 * @copyright Copyright (c) 2026
 *
 */

#ifndef APP_WHEEL_CONTROLLER_H_
#define APP_WHEEL_CONTROLLER_H_

#ifdef __cplusplus
extern "C" {
#endif

/*******************************************************************************
 * API
 ******************************************************************************/

/**
 * @brief  Main runtime supervisor booting local interfaces and starting application tasks.
 * @details Initializes encoder, motor, servo, log, and CAN peripherals, allocates 
 * FreeRTOS threads for CAN Rx (High), Drive Control (Above Normal), and CAN Tx (Normal), 
 * then hands control over to the scheduler. This call blocks permanently.
 * @return None
 */
void app_wheel_controller(void);

#ifdef __cplusplus
}
#endif

#endif /* APP_WHEEL_CONTROLLER_H_ */