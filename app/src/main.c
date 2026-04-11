#include "main.h"
#include "test_config.h"
#include "test_gpio.h"
#include "test_timer.h"

int main(void) {

    #if (GPIO_TEST == ENABLE)
    test_gpio_run();
    #endif

    #if (TIMER_TEST == ENABLE)
    test_timer_run();
    #endif

    return 0;
}