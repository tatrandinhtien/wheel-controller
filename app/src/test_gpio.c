#include "test_config.h"
#include "test_gpio.h"

#include "system_stm32f1xx.h"

#include "Driver_RCC.h"
#include "Driver_GPIO.h"


/**
 * @brief Software delay
 *
 */
static void delay(uint32_t time)
{
    while(time--)
    {
        __asm("nop");
    }
}

#ifdef  GPIO_TEST

extern ARM_DRIVER_GPIO Driver_GPIO0;

/**
 * @brief Test GPIO output.
 *
 */
static void test_blink_led()
{
    RCC_GPIOC_CLK_EN();
    Driver_GPIO0.Setup(LED, NULL);
    Driver_GPIO0.SetDirection(LED, ARM_GPIO_OUTPUT);
    Driver_GPIO0.SetOutputMode(LED, ARM_GPIO_PUSH_PULL);
    while(1)
    {        Driver_GPIO0.SetOutput(LED, ON);
        delay(DELAY_TIME);
        Driver_GPIO0.SetOutput(LED, OFF);
        delay(DELAY_TIME);
    }
}

/**
 * @brief Test GPIO input.
 *
 */
static void test_button_int()
{
    RCC_GPIOA_CLK_EN();

}

void test_gpio_run(void)
{
    RCC_SystemClock_72Mhz();
    SystemCoreClockUpdate();

    #ifdef BLINK_LED_TEST
    test_blink_led();
    #endif  /* BLINK_LED_TEST */

    #ifdef  BUTTON_INT_TEST
    test_button_int();
    #endif /* BUTTON_INTERRUPT_TEST */
}

#endif /* GPIO_TEST */