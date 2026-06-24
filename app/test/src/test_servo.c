/**
 * @file    test_servo.c
 * @author  DinhTien (tien.ta.eswe@gmail.com)
 * @brief   Testing servo module execution loops paired with FreeRTOS scheduler context active links.
 * @version 1.0
 * @date    2026-05-31
 *
 * @copyright Copyright (c) 2026
 *
 */

#include <stdio.h>
#include "cmsis_os2.h"

#include "test_servo.h"
#include "test_config.h"

#include "bsp_servo.h"
#include "bsp_log.h"

#include "Driver_Timer.h"
#include "Driver_GPIO.h"
#include "Driver_RCC.h"

#include "system_stm32f1xx.h"

/*******************************************************************************
 * Prototypes
 ******************************************************************************/

static void vTestServoTask(void *argument);

/*******************************************************************************
 * Code
 ******************************************************************************/

/**
 * @brief  Asynchronous RTOS thread executing continuous sweep cycles across the Servo's mechanical bounds.
 * @param  argument: Unused reference context payload pointer matching CMSIS prototype layout.
 * @return None
 */
static void vTestServoTask(void *argument) {
    (void)argument;

    bsp_log_printf("[Test] Servo RTOS Task Started!\r\n");

    while (1) {
        /* Sweep to Full Left Bound Position (-90.0 degrees) */
        bsp_log_printf("[Servo] Angle = -90.0 deg\r\n");
        BSP_Servo_SetAngle(-900);
        osDelay(1000);

        /* Return straight back to Center Baseline Position (0.0 degrees) */
        bsp_log_printf("[Servo] Angle = 0.0 deg\r\n");
        BSP_Servo_SetAngle(0);
        osDelay(1000);

        /* Sweep to Full Right Bound Position (+90.0 degrees) */
        bsp_log_printf("[Servo] Angle = +90.0 deg\r\n");
        BSP_Servo_SetAngle(900);
        osDelay(1000);
    }
}

void test_servo_run(void) {
    /* Boot foundational hardware abstract layer connections */
    BSP_Servo_Init();
    bsp_log_init();

    setvbuf(stdout, NULL, _IONBF, 0);

    /* Prime internal RTOS kernel structures */
    osKernelInitialize();

    const osThreadAttr_t task_attr = {
        .name = "TestServo_Task",
        .priority = osPriorityNormal,
        .stack_size = 2048
    };
    osThreadNew(vTestServoTask, NULL, &task_attr);

    /* Hand over operational execution control to the FreeRTOS Scheduler engine */
    osKernelStart();

    /* System fail-safe fallback trap catch boundary loop */
    while (1) {}
}