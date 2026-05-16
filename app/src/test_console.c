#include "test_console.h"
#include "bsp_console.h"
#include "test_config.h"

#include "test_gpio.h"

#include "system_stm32f1xx.h"

#include "Driver_GPIO.h"
#include "Driver_RCC.h"

#include <stdio.h>

extern ARM_DRIVER_GPIO Driver_GPIO0;
extern ARM_DRIVER_RCC Driver_RCC0;

/**
 * @brief Software delay
 */
static void delay(uint32_t time) {
    while (time--) {
        __asm("nop");
    }
}

void test_console_run(void) {
    RCC_GPIOC_CLK_EN();
    Driver_GPIO0.Setup(LED, NULL);
    Driver_GPIO0.SetDirection(LED, ARM_GPIO_OUTPUT);
    Driver_GPIO0.SetOutputMode(LED, ARM_GPIO_PUSH_PULL);

    BSP_Console_Init();

    // Disable stdout buffering for embedded systems
    setvbuf(stdout, NULL, _IONBF, 0);

    // 2. Vòng lặp test in liên tục
    while (1) {

        printf("Hello heo Peppa <3\r\n");

        delay(50000);
    }
}