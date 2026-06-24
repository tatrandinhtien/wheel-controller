/**
 * @file    bsp_config.c
 * @author  DinhTien (tien.ta.eswe@gmail.com)
 * @brief   Board Support Package (BSP) master initialization implementation.
 * @version 1.0
 * @date    2026-05-30
 *
 * @copyright Copyright (c) 2026
 *
 */

#include "bsp_config.h"
#include "bsp_motor.h"
#include "bsp_encoder.h"
#include "bsp_servo.h"
#include "bsp_log.h"
#include "bsp_can.h"

#include "Driver_GPIO.h"
#include "Driver_RCC.h"

#include <stddef.h>

/*******************************************************************************
 * Variables
 ******************************************************************************/

extern ARM_DRIVER_GPIO Driver_GPIO0; 
extern ARM_DRIVER_RCC  Driver_RCC0;  

/*******************************************************************************
 * Code
 ******************************************************************************/

void BSP_Init(void) {
    /* Step 1: Scale the core clock tree up to target 72MHz */
    Driver_RCC0.SetSystemClock();
    
    /* Step 2: Update CMSIS system core clock global tracking variable */
    SystemCoreClockUpdate();

    /* Step 3: Initialize the on-board Status Indicator LED (PC13) */
    Driver_GPIO0.Setup(LED, NULL);
    Driver_GPIO0.SetDirection(LED, ARM_GPIO_OUTPUT);
    Driver_GPIO0.SetOutputMode(LED, ARM_GPIO_PUSH_PULL);
    
    /* Turn on LED to visually confirm boot routine completion */
    Driver_GPIO0.SetOutput(LED, ON);
}