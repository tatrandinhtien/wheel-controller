/**
 * @file    Driver_RCC.h
 * @author  DinhTien (tien.ta.eswe@gmail.com)
 * @brief   Reset and Clock Control (RCC) Driver definitions.
 * @details Provides configuration macros for enabling peripheral clocks 
 * (GPIO, Timers, AFIO, USART) and mapping interfaces for 
 * system clock initialization on STM32F103 MCU.
 * @version 1.0
 * @date    2026-04-01
 *
 * @copyright Copyright (c) 2026
 *
 */

#ifndef DRIVER_RCC_H_
#define DRIVER_RCC_H_

#ifdef  __cplusplus
extern "C"
{
#endif

#include "Driver_Common.h"
#include "stm32f103xb.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/

#define SYSCLOCK_Mhz                72U     /**< Expected core system clock frequency in MHz. */

/* Peripheral Clock Enable Macros */
#define RCC_AFIO_CLK_EN()           (RCC->APB2ENR |= (1U << RCC_APB2ENR_AFIOEN_Pos))  /**< Enable Alternate Function IO clock. */

#define RCC_GPIOA_CLK_EN()          (RCC->APB2ENR |= (1U << RCC_APB2ENR_IOPAEN_Pos))  /**< Enable GPIO Port A clock. */
#define RCC_GPIOB_CLK_EN()          (RCC->APB2ENR |= (1U << RCC_APB2ENR_IOPBEN_Pos))  /**< Enable GPIO Port B clock. */
#define RCC_GPIOC_CLK_EN()          (RCC->APB2ENR |= (1U << RCC_APB2ENR_IOPCEN_Pos))  /**< Enable GPIO Port C clock. */

#define RCC_TIM1_CLK_EN()           (RCC->APB2ENR |= (1U << RCC_APB2ENR_TIM1EN_Pos))  /**< Enable Advanced Control Timer 1 clock. */
#define RCC_TIM2_CLK_EN()           (RCC->APB1ENR |= (1U << RCC_APB1ENR_TIM2EN_Pos))  /**< Enable General Purpose Timer 2 clock. */
#define RCC_TIM3_CLK_EN()           (RCC->APB1ENR |= (1U << RCC_APB1ENR_TIM3EN_Pos))  /**< Enable General Purpose Timer 3 clock. */
#define RCC_TIM4_CLK_EN()           (RCC->APB1ENR |= (1U << RCC_APB1ENR_TIM4EN_Pos))  /**< Enable General Purpose Timer 4 clock. */

#define RCC_USART1_EN()             (RCC->APB2ENR |= (1U << RCC_APB2ENR_USART1EN_Pos)) /**< Enable USART1 peripheral clock. */

/**
 * @brief  Access structure of the RCC Driver.
 */
typedef struct
{
    /**
     * @brief  Configures the main system clock distribution (PLL, HSE, HSI).
     * @retval ARM_DRIVER_OK: Clock configuration set successfully.
     * @retval ARM_DRIVER_ERROR: Hardware failure or timeout during clock switch.
     */
    int32_t (*SetSystemClock) (void);
} ARM_DRIVER_RCC;

#ifdef  __cplusplus
}
#endif

#endif /* DRIVER_RCC_H_ */
