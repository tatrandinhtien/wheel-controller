/**
 * @file    main.c
 * @author  DinhTien (tien.ta.eswe@gmail.com)
 * @brief   Master entry point orchestrating firmware compilation branches.
 * @version 1.0
 * @date    2026-06-23
 *
 * @copyright Copyright (c) 2026
 *
 */

#include "cmsis_os2.h"

#include "app_wheel_controller.h"

#include "test_config.h"
#include "test_gpio.h"
#include "test_log.h"
#include "test_can.h"
#include "test_servo.h"
#include "test_motor.h"
#include "test_encoder.h"

#include "bsp_can.h"
#include "bsp_encoder.h"
#include "bsp_motor.h"
#include "bsp_servo.h"
#include "bsp_log.h"

int main(void) {
    /* Enable Global Mask Interrupts for ARM Cortex-M core processing */
    __enable_irq();

    /* Trigger master clock tree initialization and default indicators setup */
    BSP_Init();

    #if (GPIO_TEST == ENABLE)
    test_gpio_run();
    #endif

    #if (LOG_TEST == ENABLE)
    test_log_run();
    #endif

    #if (CAN_TEST == ENABLE)
    test_can_run();
    #endif

    #if (SERVO_TEST == ENABLE)
    test_servo_run();
    #endif

    #if (MOTOR_TEST == ENABLE)
    test_motor_run();
    #endif

    #if (ENCODER_TEST == ENABLE)
    test_encoder_control_run();
    #endif

    #if (APP_RUN == ENABLE)
    /* Route execution directly to the distributed smart wheel node supervisor */
    app_wheel_controller();
    #endif

    return 0;
}
