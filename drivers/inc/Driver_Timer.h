/**
 * @file    Driver_Timer.h
 * @author  DinhTien (tien.ta.eswe@gmail.com)
 * @brief   Timer Peripheral Driver definitions for PWM and Encoder modes.
 * @details This file defines the hardware abstraction layer (HAL) interfaces
 * for STM32F103 Timers, split into two distinct access structures:
 * one for PWM output generation and another for Quadrature Encoder tracking.
 * @version 1.0
 * @date    2026-06-23
 * * @copyright Copyright (c) 2026
 * */

#ifndef DRIVER_TIMER_H_
#define DRIVER_TIMER_H_

#ifdef __cplusplus
extern "C"
{
#endif

#include "Driver_Common.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/

/**
 * @brief  Hardware Timer instance enumerations.
 */
typedef enum
{
    ARM_TIM_1 = 1,  /**< Advanced Control Timer 1. */
    ARM_TIM_2,      /**< General Purpose Timer 2. */
    ARM_TIM_3,      /**< General Purpose Timer 3. */
    ARM_TIM_4       /**< General Purpose Timer 4. */
} ARM_TIM_NUM;

/**
 * @brief  Timer Channel enumerations.
 */
typedef enum
{
    ARM_CHANNEL_1 = 1,  /**< Timer Channel 1 index. */
    ARM_CHANNEL_2,      /**< Timer Channel 2 index. */
    ARM_CHANNEL_3,      /**< Timer Channel 3 index. */
    ARM_CHANNEL_4       /**< Timer Channel 4 index. */
} ARM_TIM_CHANNEL;

/**
 * @brief  Encoder counting edge mode configurations.
 */
typedef enum
{
    ARM_UP_EDGE,        /**< Count on rising edges only. */
    ARM_DOWN_EDGE,      /**< Count on falling edges only. */
    ARM_EITHER_EDGE     /**< Count on both rising and falling edges (X4 mode). */
} ARM_ENCODER_MODE;

/**
 * @brief  Access structure of the Timer PWM Driver.
 */
typedef struct
{
    /**
     * @brief  Initializes the specified timer base frequency.
     * @param  tim: Timer peripheral instance (@ref ARM_TIM_NUM).
     * @param  freq_hz: Desired PWM frequency in Hertz.
     * @retval ARM_DRIVER_OK: Configuration written successfully.
     * @retval ARM_DRIVER_ERROR: Timer instance not supported or initialization failed.
     */
    int32_t (*Setup)   (ARM_TIM_NUM tim, uint16_t freq_hz);

    /**
     * @brief  Configures a specific timer channel for PWM Output Compare mode.
     * @param  tim: Timer peripheral instance (@ref ARM_TIM_NUM).
     * @param  channel: Channel index to configure (@ref ARM_TIM_CHANNEL).
     * @retval ARM_DRIVER_OK: Channel set to PWM mode successfully.
     */
    int32_t (*SetMode) (ARM_TIM_NUM tim, ARM_TIM_CHANNEL channel);

    /**
     * @brief  Updates the PWM duty cycle for a specific channel.
     * @param  tim: Timer peripheral instance (@ref ARM_TIM_NUM).
     * @param  channel: Channel index to update (@ref ARM_TIM_CHANNEL).
     * @param  duty: Duty cycle percentage or raw CCR register value (scaled appropriately).
     * @retval ARM_DRIVER_OK: Duty cycle updated successfully.
     */
    int32_t (*SetDuty) (ARM_TIM_NUM tim, ARM_TIM_CHANNEL channel, uint16_t duty);

    /**
     * @brief  Triggers/Enables PWM output generation on the target channel.
     * @param  tim: Timer peripheral instance (@ref ARM_TIM_NUM).
     * @param  channel: Channel index to enable output state.
     * @retval ARM_DRIVER_OK: PWM counter started successfully.
     */
    int32_t (*Trigger) (ARM_TIM_NUM tim, ARM_TIM_CHANNEL channel);
} ARM_DRIVER_TIM_PWM;

/**
 * @brief  Access structure of the Timer Encoder Driver.
 */
typedef struct
{
    /**
     * @brief  Initializes the timer peripheral base registers for encoder interface mode.
     * @param  tim: Timer peripheral instance (@ref ARM_TIM_NUM).
     * @retval ARM_DRIVER_OK: Initialization complete.
     */
    int32_t (*Setup) (ARM_TIM_NUM tim);

    /**
     * @brief  Sets the edge trigger sensitivity for the encoder interface.
     * @param  tim: Timer peripheral instance (@ref ARM_TIM_NUM).
     * @param  mode: Selected counting edge mode enum (@ref ARM_ENCODER_MODE).
     * @retval ARM_DRIVER_OK: Counting mode set successfully.
     */
    int32_t (*SetMode) (ARM_TIM_NUM tim, ARM_ENCODER_MODE mode);

    /**
     * @brief  Retrieves the current raw counter value from the Timer Counter register (CNT).
     * @param  tim: Timer peripheral instance (@ref ARM_TIM_NUM).
     * @return int32_t: Current accumulated encoder step count.
     */
    int32_t (*GetCount) (ARM_TIM_NUM tim);

    /**
     * @brief  Reads the current rotation direction of the encoder.
     * @param  tim: Timer peripheral instance (@ref ARM_TIM_NUM).
     * @return int32_t: Returns 1 for clockwise/forward direction, 0 for counter-clockwise/reverse.
     */
    int32_t (*GetDir) (ARM_TIM_NUM tim);
} ARM_DRIVER_TIM_ENCODER;

#ifdef __cplusplus
}
#endif

#endif /* DRIVER_TIMER_H_ */
