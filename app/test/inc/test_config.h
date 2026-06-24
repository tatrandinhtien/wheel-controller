/**
 * @file    test_config.h
 * @author  DinhTien (tien.ta.eswe@gmail.com)
 * @brief   Global Feature Toggle Profile and unit test configuration switches.
 * @details Centralizes compilation switches to selectively toggle bare-metal driver 
 * validation tests, RTOS independent subsystem components testing, or fully engage 
 * the main integrated application loop context.
 * @version 1.0
 * @date    2026-06-24
 *
 * @copyright Copyright (c) 2026
 *
 */

#ifndef TEST_CONFIG_H_
#define TEST_CONFIG_H_

#include "test_gpio.h"
#include "test_log.h"
#include "bsp_config.h"

#include "Driver_RCC.h"
#include "Driver_GPIO.h"
#include "Driver_Timer.h"

#include "system_stm32f1xx.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/*******************************************************************************
 * Definitions
 ******************************************************************************/

/**
 * @brief Master switch and sub-scenarios for low-level Bare-metal GPIO evaluation profiles.
 */
#define GPIO_TEST           DISABLE         /**< Master switch to validate standalone physical GPIO routines. */
#define BLINK_LED_TEST      DISABLE         /**< Sub-switch to isolate polled output square wave blinking logic (PC13). */
#define BUTTON_INT_TEST     DISABLE         /**< Sub-switch to isolate dual-edge asynchronous pin EXTI interrupt lines (PA3). */

/**
 * @brief Switch for asynchronous RTOS thread-safe terminal telemetry print routines.
 */
#define LOG_TEST            DISABLE         /**< Enables the FreeRTOS Queue-driven consumer log stream tracking loop. */

/**
 * @brief Switch for asynchronous FreeRTOS-managed bxCAN bus communication loops.
 */
#define CAN_TEST            DISABLE         /**< Enables localized round-robin frame Tx task and ISR RX tracking loops. */

/**
 * @brief Switch for standard 50Hz RC Servo steering mechanism angle sweeps.
 */
#define SERVO_TEST          DISABLE         /**< Enables localized angular sweep tracking sequences using TIM4_CH1. */

/**
 * @brief Switch for dynamic speed steps evaluation over the physical H-Bridge motor line.
 */
#define MOTOR_TEST          DISABLE         /**< Enables forward and reverse open-loop velocity duty cycle sweeps using TIM1_CH1. */

/**
 * @brief Switch for quadrature encoder tracking validation loops.
 */
#define ENCODER_TEST        DISABLE         /**< Enables raw edge integration odometer tests using TIM3 input capture. */

/**
 * @brief Master application profile execution toggle.
 */
#define APP_RUN             ENABLE          /**< Bypasses separate component tests to execute the fully integrated closed-loop vehicle runtime environment. */

#ifdef __cplusplus
}
#endif

#endif /* TEST_CONFIG_H_ */