/**
 * @file    Driver_RCC.c
 * @author  DinhTien (tien.ta.eswe@gmail.com)
 * @brief   Reset and Clock Control (RCC) Driver implementation.
 * @details Configures the main system clock (SYSCLK) to 72MHz using the External 
 * High-Speed oscillator (HSE) as the PLL reference source. This implementation 
 * manages Flash prefetch buffer, latency wait states, and AHB/APB peripheral 
 * bus prescalers for the STM32F103 MCU.
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

/**
 * @brief  Initializes the system clock to 72 MHz using HSE and PLL.
 * @note   Configures AHB to 72MHz, APB1 to 36MHz (max), and APB2 to 72MHz.
 * Ensures Flash access latency is adjusted to 2 wait states before clock switch.
 * @retval ARM_DRIVER_OK: System clock initialized successfully at 72MHz.
 */
int32_t RCC_SystemClock_72Mhz(void) {
    int res = ARM_DRIVER_OK;

    /* Turn on HSE and wait until it is stable */
    RCC->CR |= (1U << RCC_CR_HSEON_Pos);
    while (!((RCC->CR & (1U << RCC_CR_HSERDY_Pos)) >> RCC_CR_HSERDY_Pos))
        ;

    /* Enable the Flash prefetch buffer */
    FLASH->ACR |= (1U << FLASH_ACR_PRFTBE_Pos);
    while (!((FLASH->ACR & (1U << FLASH_ACR_PRFTBS_Pos)) >> FLASH_ACR_PRFTBS_Pos))
        ;

    /* Set Flash Access Latency to 2 wait states (required for 48MHz < SYSCLK <= 72MHz) */
    FLASH->ACR &= ~(0x7U << FLASH_ACR_LATENCY_Pos);
    FLASH->ACR |= (0b010U << FLASH_ACR_LATENCY_Pos);

    /* Configure Bus Prescalers: AHB prescaler: 1, APB1 prescaler: 2 (max 36MHz), APB2 prescaler: 1 */
    RCC->CFGR &= ~(0xFU << RCC_CFGR_HPRE_Pos);
    RCC->CFGR &= ~(0x7U << RCC_CFGR_PPRE1_Pos);
    RCC->CFGR |= (0b100U << RCC_CFGR_PPRE1_Pos);
    RCC->CFGR &= ~(0x7U << RCC_CFGR_PPRE2_Pos);

    /* Configure PLL: Select HSE as source, disable HSE division (PLLXTPRE=0), set Multiplier to X9 (8MHz * 9 = 72MHz) */
    RCC->CFGR &= ~(1U << RCC_CFGR_PLLXTPRE_Pos);
    RCC->CFGR |= (1U << RCC_CFGR_PLLSRC_Pos);
    RCC->CFGR &= ~(0xFU << RCC_CFGR_PLLMULL_Pos);
    RCC->CFGR |= (0b111U << RCC_CFGR_PLLMULL_Pos);

    /* Enable PLL and wait until it is locked/ready */
    RCC->CR |= (1U << RCC_CR_PLLON_Pos);
    while (!((RCC->CR & (1U << RCC_CR_PLLRDY_Pos)) >> RCC_CR_PLLRDY_Pos))
        ;

    /* Switch System Clock source to PLL */
    RCC->CFGR &= ~(0x3U << RCC_CFGR_SW_Pos);
    RCC->CFGR |= (0b10U << RCC_CFGR_SW_Pos);
    while (((RCC->CFGR & (0x3 << RCC_CFGR_SWS_Pos)) >> RCC_CFGR_SWS_Pos) != 0b10)
        ;

    return res;
}

/**
 * @brief Global structure instance for binding RCC Driver capabilities.
 */
ARM_DRIVER_RCC Driver_RCC0 = {RCC_SystemClock_72Mhz};
