/**
 * @file    bsp_encoder.c
 * @author  DinhTien (tien.ta.eswe@gmail.com)
 * @brief   Board Support Package (BSP) for Quadrature Encoder implementation.
 * @version 1.0
 * @date    2026-05-30
 *
 * @copyright Copyright (c) 2026
 *
 */

#include "bsp_encoder.h"
#include "bsp_config.h"
#include "Driver_Timer.h"
#include "Driver_GPIO.h"

/*******************************************************************************
 * Variables
 ******************************************************************************/

extern ARM_DRIVER_TIM_ENCODER Driver_TIM1; 
extern ARM_DRIVER_GPIO        Driver_GPIO0;

static uint16_t prev_count = 0U;            
static int32_t  total_position = 0;         

/*******************************************************************************
 * Code
 ******************************************************************************/

void BSP_Encoder_Init(void) {
    /* Set up phase channel A pin input line configurations */
    Driver_GPIO0.Setup(MOTOR_ENCODER_A, NULL);
    Driver_GPIO0.SetDirection(MOTOR_ENCODER_A, ARM_GPIO_INPUT);
    Driver_GPIO0.SetPullResistor(MOTOR_ENCODER_A, ARM_GPIO_PULL_NONE);

    /* Set up phase channel B pin input line configurations */
    Driver_GPIO0.Setup(MOTOR_ENCODER_B, NULL);
    Driver_GPIO0.SetDirection(MOTOR_ENCODER_B, ARM_GPIO_INPUT);
    Driver_GPIO0.SetPullResistor(MOTOR_ENCODER_B, ARM_GPIO_PULL_NONE);

    /* Configure Timer 3 peripheral to drive hardware Quadrature Encoder mode */
    Driver_TIM1.Setup(ARM_TIM_3);
    Driver_TIM1.SetMode(ARM_TIM_3, ARM_EITHER_EDGE);

    /* Reset global tracking runtime variable states back to baseline */
    prev_count = 0U;
    total_position = 0;
}

int16_t BSP_Encoder_GetDelta(void) {
    uint16_t current_count;
    int16_t delta;

    /* Fetch current raw hardware register tick status */
    current_count = (uint16_t)Driver_TIM1.GetCount(ARM_TIM_3);
    
    /* Two's Complement signed integer casting resolves register rollovers naturally */
    delta = (int16_t)(current_count - prev_count);
    
    /* Refresh tracking index timeline points */
    prev_count = current_count;
    
    /* Accumulate fractional tracking delta updates into 32-bit absolute position ledger */
    total_position += delta;

    return delta;
}

int32_t BSP_Encoder_GetTotalPosition(void) {
    return total_position;
}