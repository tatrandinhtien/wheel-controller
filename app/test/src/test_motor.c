/**
 * @file    test_motor.c
 * @author  DinhTien (tien.ta.eswe@gmail.com)
 * @brief   Testing H-Bridge motor control loops paired with FreeRTOS scheduler context.
 * @version 1.0
 * @date    2026-05-31
 *
 * @copyright Copyright (c) 2026
 *
 */

#include <stdio.h>
#include "cmsis_os2.h"

#include "test_motor.h"
#include "test_config.h"

#include "bsp_motor.h"
#include "bsp_log.h"

#include "Driver_Timer.h"
#include "Driver_GPIO.h"
#include "Driver_RCC.h"

#include "system_stm32f1xx.h"

/*******************************************************************************
 * Prototypes
 ******************************************************************************/

static void vTestMotorTask(void *argument);

/*******************************************************************************
 * Code
 ******************************************************************************/

/**
 * @brief  Asynchronous RTOS thread executing a dynamic velocity loop to test the H-Bridge.
 * @param  argument: Unused reference context payload pointer matching CMSIS prototype layout.
 * @return None
 */
static void vTestMotorTask(void *argument) {
    (void)argument;

    bsp_log_printf("[Test] Motor RTOS Task Started!\r\n");

    while (1) {
        /* Phase 1: Gradual Forward Acceleration Step Testing */
        bsp_log_printf("[Motor] Forwarding: Speed 30\r\n");
        BSP_Motor_SetSpeed(30);
        osDelay(2000);

        bsp_log_printf("[Motor] Forwarding: Speed 70\r\n");
        BSP_Motor_SetSpeed(70);
        osDelay(2000);

        bsp_log_printf("[Motor] Forwarding: Speed 99\r\n");
        BSP_Motor_SetSpeed(99);
        osDelay(2000);

        bsp_log_printf("[Motor] Decelerating: Speed 40\r\n");
        BSP_Motor_SetSpeed(40);
        osDelay(1000);

        /* Phase 2: Neutral Coasting Stop (Dissipate kinetic energy safely) */
        bsp_log_printf("[Motor] Coasting Stop...\r\n");
        BSP_Motor_SetSpeed(0);
        osDelay(1500);

        /* Phase 3: Reverse Direction Step Testing */
        bsp_log_printf("[Motor] Reversing: Speed -30\r\n");
        BSP_Motor_SetSpeed(-30);
        osDelay(2000);

        bsp_log_printf("[Motor] Reversing: Speed -70\r\n");
        BSP_Motor_SetSpeed(-70);
        osDelay(2000);

        /* Phase 4: Main Idle Loop Reset Boundary */
        bsp_log_printf("[Motor] Full Idle Stop\r\n");
        BSP_Motor_SetSpeed(0);
        osDelay(3000);
    }
}

void test_motor_run(void) {
    /* Boot foundational hardware abstract layer connections */
    BSP_Motor_Init();
    bsp_log_init();

    setvbuf(stdout, NULL, _IONBF, 0);

    /* Prime internal RTOS kernel structures */
    osKernelInitialize();

    const osThreadAttr_t task_attr = {
        .name = "TestMotor_Task",
        .priority = osPriorityNormal,
        .stack_size = 2048
    };
    osThreadNew(vTestMotorTask, NULL, &task_attr);

    /* Hand over operational execution control to the FreeRTOS Scheduler engine */
    osKernelStart();

    /* System fail-safe fallback trap catch boundary loop */
    while (1) {}
}