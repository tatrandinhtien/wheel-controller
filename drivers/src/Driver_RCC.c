/**
 * @file Driver_RCC.c
 * @author Tien Ta (tien.ta.eswe@gmail.com)
 * @brief
 * @version 0.1
 * @date 2026-04-01
 *
 * @copyright Copyright (c) 2026
 *
 */

#include "Driver_RCC.h"
#include "stm32f103xb.h"

/**
 * @brief Configure the SYSCLK up to 72 Mhz
 *
 */
int RCC_SystemClock_72Mhz(void)
{
    int res = ARM_DRIVER_OK;

    /* Turn on HSE and wait until its stable */
    RCC->CR |= (1U << RCC_CR_HSEON_Pos);
    while(!((RCC->CR & (1U << RCC_CR_HSERDY_Pos)) >> RCC_CR_HSERDY_Pos));

    /* Enable the prefetch buffer */
    FLASH->ACR |= (1U << FLASH_ACR_PRFTBE_Pos);
    while(!((FLASH->ACR & (1U << FLASH_ACR_PRFTBS_Pos)) >> FLASH_ACR_PRFTBS_Pos));

    /* 2 wait states */
    FLASH->ACR &= ~(0x7U << FLASH_ACR_LATENCY_Pos);
    FLASH->ACR |= (0b010U << FLASH_ACR_LATENCY_Pos);

    /* AHB prescaler 1, APB1 prescaler 2, APB2 prescaler 1 */
    RCC->CFGR &= ~(0xFU << RCC_CFGR_HPRE_Pos);
    RCC->CFGR &= ~(0x7U << RCC_CFGR_PPRE1_Pos);
    RCC->CFGR |= (0b100U << RCC_CFGR_PPRE1_Pos);
    RCC->CFGR &= ~(0x7U << RCC_CFGR_PPRE2_Pos);

    /* Configure PLL */
    RCC->CFGR &= ~(1U << RCC_CFGR_PLLXTPRE_Pos);
    RCC->CFGR |= (1U << RCC_CFGR_PLLSRC_Pos);
    RCC->CFGR &= ~(0xFU << RCC_CFGR_PLLMULL_Pos);
    RCC->CFGR |= (0b111U << RCC_CFGR_PLLMULL_Pos);

    /* Enable PLL */
    RCC->CR |= (1U << RCC_CR_PLLON_Pos);
    while(!((RCC->CR & (1U << RCC_CR_PLLRDY_Pos)) >> RCC_CR_PLLRDY_Pos));

    /* System clock switch */
    RCC->CFGR &= ~(0x3U << RCC_CFGR_SW_Pos);
    RCC->CFGR |= (0b10U << RCC_CFGR_SW_Pos);
    while(((RCC->CFGR & (0x3 << RCC_CFGR_SWS_Pos)) >> RCC_CFGR_SWS_Pos) != 0b10);

    return res;
}