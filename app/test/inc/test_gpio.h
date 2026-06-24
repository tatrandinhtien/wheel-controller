/**
 * @file    test_gpio.h
 * @author  DinhTien (tien.ta.eswe@gmail.com)
 * @brief   Testing framework interface for GPIO inputs, outputs, and interrupts.
 * @version 1.0
 * @date    2026-06-24
 *
 * @copyright Copyright (c) 2026
 *
 */

#ifndef TEST_GPIO_H
#define TEST_GPIO_H

/*******************************************************************************
 * API
 ******************************************************************************/

/**
 * @brief  Main entry point executing selected low-level GPIO hardware validation blocks.
 * @note   Depending on build configuration macros (BLINK_LED_TEST or BUTTON_INT_TEST), 
 * this routine will branch into an infinite test loop and will not return.
 * @return None
 */
void test_gpio_run(void);

#endif /* TEST_GPIO_H */