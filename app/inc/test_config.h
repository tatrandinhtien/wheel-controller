#ifndef TEST_CONFIG_H
#define TEST_CONFIG_H

#include "test_gpio.h"
#include "test_timer.h"

#include "system_stm32f1xx.h"

#include "Driver_RCC.h"
#include "Driver_GPIO.h"
#include "Driver_Timer.h"

/* PC13 */
#define LED                 45

/* PA10 */
#define BUTTON              10

#define ENABLE              1
#define DISABLE             0

#define ON                  0
#define OFF                 1

#define HIGH                1
#define LOW                 0

#define DELAY_TIME          5000000

#define GPIOA_PIN_8         8
#define GPIOA_PIN_0         0
#define GPIOA_PIN_1         1

#define GPIOB_PIN_14        30
#define GPIOB_PIN_15        31

#define MOTOR_PWM_PIN       GPIOA_PIN_8
#define MOTOR_ENCODER_A     GPIOA_PIN_0
#define MOTOR_ENCODER_B     GPIOA_PIN_1

#define MOTOR_IN1           GPIOB_PIN_14
#define MOTOR_IN2           GPIOB_PIN_15

#define PWM_FREQ            20000

/**
 * @brief GPIO constant
 */
#define GPIO_TEST           ENABLE
#define BLINK_LED_TEST      ENABLE
#define BUTTON_INT_TEST     ENABLE

/**
 * @brief Timer constant
 */
#define TIMER_TEST          DISABLE
#define CONTROL_MOTOR_TEST  ENABLE
#define ENCODER_MOTOR_TEST  ENABLE

#endif /* TEST_CONFIG_H */