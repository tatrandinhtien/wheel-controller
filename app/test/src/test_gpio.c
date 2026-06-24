/**
 * @file    test_gpio.c
 * @author  DinhTien (tien.ta.eswe@gmail.com)
 * @brief   Testing peripheral drivers for GPIO toggling and external interrupt tracking.
 * @version 1.0
 * @date    2026-05-31
 *
 * @copyright Copyright (c) 2026
 *
 */

#include "test_gpio.h"
#include "test_config.h"

#include "Driver_GPIO.h"
#include "Driver_RCC.h"

#include "system_stm32f1xx.h"

#ifdef GPIO_TEST

/*******************************************************************************
 * Variables
 ******************************************************************************/

extern ARM_DRIVER_GPIO Driver_GPIO0;
extern ARM_DRIVER_RCC  Driver_RCC0;

/*******************************************************************************
 * Prototypes
 ******************************************************************************/

static void delay(uint32_t time);
static void test_blink_led(void);
static void test_button_int(void);
void button_callback(ARM_GPIO_Pin_t pin, uint32_t event);

/*******************************************************************************
 * Code
 ******************************************************************************/

/**
 * @brief  Executes a raw CPU pipeline stall loop via Assembly NOP operations.
 * @param  time: Number of iterations to cycle through.
 * @return None
 */
static void delay(uint32_t time) {
    while (time--) {
        __asm("nop");
    }
}

/**
 * @brief  Asynchronous hardware ISR callback handler invoked upon external signal switches.
 * @param  pin: Mapped physical index target identification token.
 * @param  event: Fired operational edge pattern mask signal type.
 * @return None
 */
void button_callback(ARM_GPIO_Pin_t pin, uint32_t event) {
    if (pin == BUTTON) {
        /* Active-low status LED: Turn OFF when button is released, ON when pressed */
        if (event == ARM_GPIO_TRIGGER_RISING_EDGE) {
            Driver_GPIO0.SetOutput(LED, OFF);
        } else if (event == ARM_GPIO_TRIGGER_FALLING_EDGE) {
            Driver_GPIO0.SetOutput(LED, ON);
        }
    }
}

/**
 * @brief  Polled loop sequence continuously toggling output logical states.
 * @return None
 */
static void test_blink_led(void) {
    while (1) {
        Driver_GPIO0.SetOutput(LED, ON);
        delay(DELAY_TIME);
        Driver_GPIO0.SetOutput(LED, OFF);
        delay(DELAY_TIME);
    }
}

/**
 * @brief  Configures input pull-up lines paired with nested edge interrupts.
 * @return None
 */
static void test_button_int(void) {
    /* Step 1: Clock gating enable and route input pin lines (PA3) to external interrupt triggers */
    RCC_GPIOA_CLK_EN();
    Driver_GPIO0.Setup(BUTTON, button_callback);
    Driver_GPIO0.SetDirection(BUTTON, ARM_GPIO_INPUT);
    Driver_GPIO0.SetPullResistor(BUTTON, ARM_GPIO_PULL_UP);
    Driver_GPIO0.SetEventTrigger(BUTTON, ARM_GPIO_TRIGGER_EITHER_EDGE);

    /* Step 2: Clock gating enable and configure active status indicator output line (PC13) */
    RCC_GPIOC_CLK_EN();
    Driver_GPIO0.Setup(LED, NULL);
    Driver_GPIO0.SetDirection(LED, ARM_GPIO_OUTPUT);
    Driver_GPIO0.SetOutputMode(LED, ARM_GPIO_PUSH_PULL);

    while (1) {
        /* Trap execution context; state transitions are handled asynchronously inside the ISR */
    }
}

void test_gpio_run(void) {
#ifdef BLINK_LED_TEST
    test_blink_led();
#endif /* BLINK_LED_TEST */

#ifdef BUTTON_INT_TEST
    test_button_int();
#endif /* BUTTON_INT_TEST */
}

#endif /* GPIO_TEST */