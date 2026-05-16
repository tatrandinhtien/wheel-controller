#ifndef TEST_CONFIG_H
#define TEST_CONFIG_H

#include "test_gpio.h"
#include "test_timer.h"
#include "test_console.h"

#include "bsp_config.h"

#include "Driver_RCC.h"
#include "Driver_GPIO.h"
#include "Driver_Timer.h"

#include "system_stm32f1xx.h"

/**
 * @brief GPIO constant
 */
#define GPIO_TEST           DISABLE
#define BLINK_LED_TEST      DISABLE
#define BUTTON_INT_TEST     DISABLE

/**
 * @brief Timer constant
 */
#define TIMER_TEST          DISABLE
#define CONTROL_MOTOR_TEST  DISABLE
#define ENCODER_MOTOR_TEST  DISABLE

/**
 * @brief Console constant
 */
#define CONSOLE_TEST        DISABLE

/**
 * @brief Console constant
 */
#define CAN_TEST            ENABLE

#endif /* TEST_CONFIG_H */