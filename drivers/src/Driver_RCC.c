/**
 * @file    Driver_RCC.c
 * @author  DinhTien (tien.ta.eswe@gmail.com)
 * @brief   Reset and Clock Control (RCC) Driver implementation.
 * @version 1.0
 * @date    2026-04-01
 *
 * @copyright Copyright (c) 2026
 *
 */

#include "Driver_RCC.h"
#include "stm32f103xb.h"

/*******************************************************************************
 * Code
 ******************************************************************************/

int32_t RCC_SystemClock_72Mhz(void) {
    int res = ARM_DRIVER_OK;

    /* Step 1: Turn on HSE and wait until it is stable */
    RCC->CR |= (1U << RCC_CR_HSEON_Pos);
    while (!((RCC->CR & (1U << RCC_CR_HSERDY_Pos)) >> RCC_CR_HSERDY_Pos))
        ;

    /* Step 2: Enable the Flash prefetch buffer */
    FLASH->ACR |= (1U << FLASH_ACR_PRFTBE_Pos);
    while (!((FLASH->ACR & (1U << FLASH_ACR_PRFTBS_Pos)) >> FLASH_ACR_PRFTBS_Pos))
        ;

    /* Step 3: Set Flash Access Latency to 2 wait states (required for 48MHz < SYSCLK <= 72MHz) */
    FLASH->ACR &= ~(0x7U << FLASH_ACR_LATENCY_Pos);
    FLASH->ACR |= (0b010U << FLASH_ACR_LATENCY_Pos);

    /* Step 4: Configure Bus Prescalers: AHB = 1, APB1 = 2 (max 36MHz), APB2 = 1 */
    RCC->CFGR &= ~(0xFU << RCC_CFGR_HPRE_Pos);
    RCC->CFGR &= ~(0x7U << RCC_CFGR_PPRE1_Pos);
    RCC->CFGR |= (0b100U << RCC_CFGR_PPRE1_Pos);
    RCC->CFGR &= ~(0x7U << RCC_CFGR_PPRE2_Pos);

    /* Step 5: Configure PLL: Select HSE as source, disable HSE division, multiplier = X9 (8MHz * 9 = 72MHz) */
    RCC->CFGR &= ~(1U << RCC_CFGR_PLLXTPRE_Pos);
    RCC->CFGR |= (1U << RCC_CFGR_PLLSRC_Pos);
    RCC->CFGR &= ~(0xFU << RCC_CFGR_PLLMULL_Pos);
    RCC->CFGR |= (0b111U << RCC_CFGR_PLLMULL_Pos);

    /* Step 6: Enable PLL and wait until it is locked/ready */
    RCC->CR |= (1U << RCC_CR_PLLON_Pos);
    while (!((RCC->CR & (1U << RCC_CR_PLLRDY_Pos)) >> RCC_CR_PLLRDY_Pos))
        ;

    /* Step 7: Switch System Clock source to PLL and verify status */
    RCC->CFGR &= ~(0x3U << RCC_CFGR_SW_Pos);
    RCC->CFGR |= (0b10U << RCC_CFGR_SW_Pos);
    while (((RCC->CFGR & (0x3 << RCC_CFGR_SWS_Pos)) >> RCC_CFGR_SWS_Pos) != 0b10)
        ;

    return res;
}

ARM_DRIVER_RCC Driver_RCC0 = {RCC_SystemClock_72Mhz};