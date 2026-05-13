#ifndef TEST_CONFIG_H
#define TEST_CONFIG_H

#include "test_gpio.h"
#include "test_timer.h"
#include "test_console.h"

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

#define PORTA_PIN8          8
#define PORTA_PIN0          0
#define PORTA_PIN1          1

#define PORTA_PIN9          9
#define PORTA_PIN10         10

#define PORTA_PIN2          2
#define PORTA_PIN3          3

#define PORTB_PIN10         26
#define PORTB_PIN11         27

#define PORTB_PIN14         30
#define PORTB_PIN15         31

#define MOTOR_PWM_PIN       PORTA_PIN8
#define MOTOR_ENCODER_A     PORTA_PIN0
#define MOTOR_ENCODER_B     PORTA_PIN1

#define MOTOR_IN1           PORTB_PIN14
#define MOTOR_IN2           PORTB_PIN15

#define USART1_TX           PORTA_PIN9
#define USART1_RX           PORTA_PIN10

#define USART2_TX           PORTA_PIN2
#define USART2_RX           PORTA_PIN3

#define USART3_TX           PORTB_PIN10
#define USART3_RX           PORTB_PIN11

#define PWM_FREQ            20000
#define DUTY_50             50



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