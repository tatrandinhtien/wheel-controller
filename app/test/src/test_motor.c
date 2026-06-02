/**
 * @file test_motor.c
 * @author dt (tien.ta.eswe@gmail.com)
 * @brief testing H-Bridge motor control module with FreeRTOS
 * @version 0.1
 * @date 2026-05-31
 * * @copyright Copyright (c) 2026
 * */
#include "test_motor.h"
#include "bsp_hbridge.h"
#include "bsp_log.h"
#include "test_config.h"

#include "system_stm32f1xx.h"
#include "Driver_Timer.h"
#include "Driver_GPIO.h"
#include "Driver_RCC.h"

#include <stdio.h>
#include "cmsis_os2.h"

static void vTestMotorTask(void *argument) {
    (void)argument;

    bsp_log_printf("[Test] Motor RTOS Task Started!\r\n");

    while (1) {
        // bsp_log_printf("[Motor] Forwarding: 30%%\r\n");
        // BSP_Motor_SetSpeed(30);
        // osDelay(2000);

        // bsp_log_printf("[Motor] Forwarding: 70%%\r\n");
        // BSP_Motor_SetSpeed(70);
        // osDelay(2000);

        // bsp_log_printf("[Motor] Forwarding: 99%%\r\n");
        // BSP_Motor_SetSpeed(99);
        // osDelay(2000);

        // bsp_log_printf("[Motor] Forwarding 40%%\r\n");
        // BSP_Motor_SetSpeed(40);
        // osDelay(1000);

        // bsp_log_printf("[Motor] Stop...\r\n");
        // BSP_Motor_SetSpeed(0);
        // osDelay(1500);

        // bsp_log_printf("[Motor] Reverse: -30%%\r\n");
        // BSP_Motor_SetSpeed(-30);
        // osDelay(2000);

        // bsp_log_printf("[Motor] Reverse: -70%%\r\n");
        // BSP_Motor_SetSpeed(-70);
        // osDelay(2000);

        bsp_log_printf("[Motor] Stop\r\n");
        BSP_Motor_SetSpeed(0);
        osDelay(3000);
    }
}

void test_motor_run(void) {

    BSP_Motor_Init();
    bsp_log_init();

    setvbuf(stdout, NULL, _IONBF, 0);

    osKernelInitialize();

    const osThreadAttr_t task_attr = {
        .name = "TestMotor_Task",
        .priority = osPriorityNormal,
        .stack_size = 2048
    };
    osThreadNew(vTestMotorTask, NULL, &task_attr);

    osKernelStart();

    while (1) {}
}