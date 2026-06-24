/**
 * @file    test_log.c
 * @author  DinhTien (tien.ta.eswe@gmail.com)
 * @brief   Testing console module logging rates paired with FreeRTOS scheduler context.
 * @version 1.0
 * @date    2026-05-31
 * * @copyright Copyright (c) 2026
 * */

#include <stdio.h>
#include "cmsis_os2.h"

#include "test_config.h"
#include "test_log.h"
#include "test_gpio.h"

#include "bsp_log.h"
#include "Driver_GPIO.h"
#include "Driver_RCC.h"
#include "system_stm32f1xx.h"

/*******************************************************************************
 * Prototypes
 ******************************************************************************/

static void vTestLogTask(void *argument);

/*******************************************************************************
 * Variables
 ******************************************************************************/

extern ARM_DRIVER_GPIO Driver_GPIO0;
extern ARM_DRIVER_RCC  Driver_RCC0;

/*******************************************************************************
 * Code
 ******************************************************************************/

/**
 * @brief  Asynchronous RTOS thread executing continuous non-blocking stream transmissions.
 * @param  argument: Unused reference context payload pointer matching CMSIS prototype layout.
 * @return None
 */
static void vTestLogTask(void *argument) {
    (void)argument;

    while (1) {
        /* Push message buffer onto background queue safely without task lock blocks */
        bsp_log_printf("This is non-blocking print !\r\n");
        osDelay(500);
    }
}

void test_log_run(void) {
    /* Boot background logging tasks, queues, and semaphores */
    bsp_log_init();
    
    /* Disable stdout buffer to prevent standard library caching side effects */
    setvbuf(stdout, NULL, _IONBF, 0);

    /* Prime internal RTOS kernel structures */
    osKernelInitialize();

    const osThreadAttr_t task_attr = {
        .name = "TestLog_Task",
        .priority = osPriorityNormal,
        .stack_size = 2048
    };
    osThreadNew(vTestLogTask, NULL, &task_attr);

    /* Hand over operational execution control to the FreeRTOS Scheduler engine */
    osKernelStart();

    /* System fail-safe fallback trap catch boundary loop */
    while (1) {}
}