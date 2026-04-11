/**
 * @file Driver_RCC.h
 * @author Tien Ta (tien.ta.eswe@gmail.com)
 * @brief APIs for clock configuration
 * @version 0.1
 * @date 2026-04-01
 *
 * @copyright Copyright (c) 2026
 *
 */

#ifndef DRIVER_RCC_H_
#define DRIVER_RCC_H_

#include "Driver_Common.h"
#include "stm32f103xb.h"

#define SYSCLOCK_Mhz                72U

#define RCC_AFIO_CLK_EN()           (RCC->APB2ENR |= (1U << RCC_APB2ENR_AFIOEN_Pos))

#define RCC_GPIOA_CLK_EN()          (RCC->APB2ENR |= (1U << RCC_APB2ENR_IOPAEN_Pos))
#define RCC_GPIOB_CLK_EN()          (RCC->APB2ENR |= (1U << RCC_APB2ENR_IOPBEN_Pos))
#define RCC_GPIOC_CLK_EN()          (RCC->APB2ENR |= (1U << RCC_APB2ENR_IOPCEN_Pos))

#define RCC_TIM1_CLK_EN()           (RCC->APB2ENR |= (1U << RCC_APB2ENR_TIM1EN_Pos))
#define RCC_TIM2_CLK_EN()           (RCC->APB1ENR |= (1U << RCC_APB1ENR_TIM2EN_Pos))
#define RCC_TIM3_CLK_EN()           (RCC->APB1ENR |= (1U << RCC_APB1ENR_TIM3EN_Pos))
#define RCC_TIM4_CLK_EN()           (RCC->APB1ENR |= (1U << RCC_APB1ENR_TIM4EN_Pos))

typedef struct
{
    int32_t (*SetSystemClock) (void);
} ARM_DRIVER_RCC;

#endif /* DRIVER_RCC_H_ */
