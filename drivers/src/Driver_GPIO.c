/**
 * @file    Driver_GPIO.c
 * @author  Tien Ta (tien.ta.eswe@gmail.com)
 * @brief   Driver for GPIO with these function:
 *          - Register Callback for interrupt.
 *          - Read status of pin.
 *          - Write value to pin.
 *          - Config Output mode push/pull or open drain.
 *          - Config Input mode pullup/pulldown.
 * @version 0.1
 * @date 2026-04-04
 *
 * @copyright Copyright (c) 2026
 *
 */

#include "Driver_GPIO.h"
#include "Driver_RCC.h"
#include "stm32f103xb.h"

#define GPIO_MAX_PINS 48U
#define PIN_IS_AVAILABLE(n) ((n) < GPIO_MAX_PINS)
#define GPIO_NUMS 3U
#define PINS_OF_PORT 16U

/********************************************************************
 * Definitions
 ********************************************************************/
typedef struct {
    GPIO_TypeDef* gpio;
    uint8_t port_num;
    uint8_t pin_num;
} GPIO_t;

static GPIO_TypeDef* gpio_arr[GPIO_NUMS] = {GPIOA, GPIOB, GPIOC};
static ARM_GPIO_SignalEvent_t gpio_callback_event[PINS_OF_PORT];
static uint8_t gpio_exti_active_pin[PINS_OF_PORT];
static GPIO_TypeDef* gpio_exti_active_port[PINS_OF_PORT];

/********************************************************************
 * Helper function
 ********************************************************************/
/**
 * @brief Reset value of GPIO struct.
 *
 */
static void GPIO_ResetStruct(GPIO_t* my_gpio) {
    my_gpio->gpio = NULL;
    my_gpio->port_num = 0;
    my_gpio->pin_num = 0;
}

/**
 * @brief Convert port, pin number, gpio register.
 *
 */
static void GPIO_ConvertPin(ARM_GPIO_Pin_t pin, GPIO_t* my_gpio) {
    if (my_gpio != NULL) {
        my_gpio->port_num = (uint8_t)pin / PINS_OF_PORT;
        my_gpio->gpio = gpio_arr[my_gpio->port_num];
        my_gpio->pin_num = (uint8_t)pin % PINS_OF_PORT;
    }
}

/********************************************************************
 * GPIO Driver Function
 ********************************************************************/
/**
 * @brief Set up GPIO interface.
 *
 */
static int32_t GPIO_Setup(ARM_GPIO_Pin_t pin, ARM_GPIO_SignalEvent_t cb_event) {
    int32_t result = ARM_DRIVER_OK;

    if (PIN_IS_AVAILABLE(pin)) {
        GPIO_t my_gpio;

        GPIO_ResetStruct(&my_gpio);
        GPIO_ConvertPin(pin, &my_gpio);

        /* Turn on clock for relative Port */
        switch (my_gpio.port_num) {
            case 0:
                RCC_GPIOA_CLK_EN();
                break;
            case 1:
                RCC_GPIOB_CLK_EN();
                break;
            case 2:
                RCC_GPIOC_CLK_EN();
                break;
            default:
                /* Can not reach here */
                break;
        }

        /* Configure Interrupt! */
        if (cb_event != NULL) {
            uint8_t exticr_reg_num = my_gpio.pin_num / 4;
            uint8_t exticr_pin_num = (my_gpio.pin_num % 4) * 4;
            IRQn_Type gpio_irqn_arr[] = {EXTI0_IRQn, EXTI1_IRQn,   EXTI2_IRQn,    EXTI3_IRQn,
                                         EXTI4_IRQn, EXTI9_5_IRQn, EXTI15_10_IRQn};

            gpio_exti_active_pin[my_gpio.pin_num] = pin;
            gpio_exti_active_port[my_gpio.pin_num] = my_gpio.gpio;

            /* Input source for EXTIx interrupt */
            RCC_AFIO_CLK_EN();
            switch (my_gpio.port_num) {
                case 0:
                    AFIO->EXTICR[exticr_reg_num] &= ~(0xF << exticr_pin_num);
                    break;
                case 1:
                    AFIO->EXTICR[exticr_reg_num] &= ~(0xF << exticr_pin_num);
                    AFIO->EXTICR[exticr_reg_num] |= (0b0001 << exticr_pin_num);
                    break;
                case 2:
                    AFIO->EXTICR[exticr_reg_num] &= ~(0xF << exticr_pin_num);
                    AFIO->EXTICR[exticr_reg_num] |= (0b0010 << exticr_pin_num);
                    break;
            }
            if (my_gpio.pin_num >= 10) {
                NVIC_EnableIRQ(gpio_irqn_arr[6]);
            } else if (my_gpio.pin_num >= 5) {
                NVIC_EnableIRQ(gpio_irqn_arr[5]);
            } else if (my_gpio.pin_num >= 0) {
                NVIC_EnableIRQ(gpio_irqn_arr[my_gpio.pin_num]);
            } else {
                result = ARM_GPIO_ERROR_PIN;
            }

            gpio_callback_event[my_gpio.pin_num] = cb_event;
        }
        /* Mask line x for safety */
        EXTI->IMR &= ~(1U << my_gpio.pin_num);
    } else {
        result = ARM_GPIO_ERROR_PIN;
    }

    return result;
}

/**
 * @brief Set GPIO direction.
 *
 */
static int32_t GPIO_SetDirection(ARM_GPIO_Pin_t pin, ARM_GPIO_DIRECTION direction) {
    int32_t result = ARM_DRIVER_OK;

    if (PIN_IS_AVAILABLE(pin)) {
        volatile uint32_t* gpio_low_high_reg = NULL;
        GPIO_t my_gpio;

        GPIO_ResetStruct(&my_gpio);
        GPIO_ConvertPin(pin, &my_gpio);

        if (my_gpio.pin_num >= 8) {
            gpio_low_high_reg = (volatile uint32_t*)&my_gpio.gpio->CRH;
        } else {
            gpio_low_high_reg = (volatile uint32_t*)&my_gpio.gpio->CRL;
        }

        switch (direction) {
            case ARM_GPIO_INPUT:
                *gpio_low_high_reg &= ~(0b11 << (my_gpio.pin_num % 8 * 4));
                break;
            case ARM_GPIO_OUTPUT:
                /* Slew rate 2Mhz */
                *gpio_low_high_reg &= ~(0b11 << (my_gpio.pin_num % 8 * 4));
                *gpio_low_high_reg |= (0b10 << (my_gpio.pin_num % 8 * 4));
                break;
            case ARM_GPIO_AF_OUTPUT:
                *gpio_low_high_reg &= ~(0b11 << (my_gpio.pin_num % 8 * 4));
                *gpio_low_high_reg |= (0b11 << (my_gpio.pin_num % 8 * 4));
                break;
            default:
                result = ARM_DRIVER_ERROR_PARAMETER;
                break;
        }
    } else {
        result = ARM_GPIO_ERROR_PIN;
    }

    return result;
}

/**
 * @brief Set GPIO output mode.
 *
 */
static int32_t GPIO_SetOutputMode(ARM_GPIO_Pin_t pin, ARM_GPIO_OUTPUT_MODE mode) {
    int32_t result = ARM_DRIVER_OK;

    if (PIN_IS_AVAILABLE(pin)) {
        volatile uint32_t* gpio_low_high_reg = NULL;
        GPIO_t my_gpio;

        GPIO_ResetStruct(&my_gpio);
        GPIO_ConvertPin(pin, &my_gpio);

        if (my_gpio.pin_num >= 8) {
            gpio_low_high_reg = (volatile uint32_t*)&my_gpio.gpio->CRH;
        } else {
            gpio_low_high_reg = (volatile uint32_t*)&my_gpio.gpio->CRL;
        }

        switch (mode) {
            case ARM_GPIO_PUSH_PULL:
                *gpio_low_high_reg &= ~(0b11 << (((my_gpio.pin_num % 8) * 4) + 2));
                break;
            case ARM_GPIO_OPEN_DRAIN:
                *gpio_low_high_reg &= ~(0b11 << (((my_gpio.pin_num % 8) * 4) + 2));
                *gpio_low_high_reg |= (0b01 << (((my_gpio.pin_num % 8) * 4) + 2));
                break;
            case ARM_AFIO_PUSH_PULL:
                *gpio_low_high_reg &= ~(0b11 << (((my_gpio.pin_num % 8) * 4) + 2));
                *gpio_low_high_reg |= (0b10 << (((my_gpio.pin_num % 8) * 4) + 2));
                break;
            case ARM_AFIO_OPEN_DRAIN:
                *gpio_low_high_reg &= ~(0b11 << (((my_gpio.pin_num % 8) * 4) + 2));
                *gpio_low_high_reg |= (0b11 << (((my_gpio.pin_num % 8) * 4) + 2));
                break;
            default:
                result = ARM_DRIVER_ERROR_PARAMETER;
                break;
        }
    } else {
        result = ARM_GPIO_ERROR_PIN;
    }

    return result;
}

/**
 * @brief Set GPIO pull resistor.
 *
 */
static int32_t GPIO_SetPullResistor(ARM_GPIO_Pin_t pin, ARM_GPIO_PULL_RESISTOR resistor) {
    int32_t result = ARM_DRIVER_OK;

    if (PIN_IS_AVAILABLE(pin)) {
        volatile uint32_t* gpio_low_high_reg = NULL;
        GPIO_t my_gpio;

        GPIO_ResetStruct(&my_gpio);
        GPIO_ConvertPin(pin, &my_gpio);

        if (my_gpio.pin_num >= 8) {
            gpio_low_high_reg = (volatile uint32_t*)&my_gpio.gpio->CRH;
        } else {
            gpio_low_high_reg = (volatile uint32_t*)&my_gpio.gpio->CRL;
        }

        switch (resistor) {
            case ARM_GPIO_PULL_NONE:
                /* Do nothing */
                break;
            case ARM_GPIO_PULL_UP:
                *gpio_low_high_reg &= ~(0b11 << (((my_gpio.pin_num % 8) * 4) + 2));
                *gpio_low_high_reg |= (0b10 << (((my_gpio.pin_num % 8) * 4) + 2));
                my_gpio.gpio->BSRR = (1U << my_gpio.pin_num);
                break;
            case ARM_GPIO_PULL_DOWN:
                *gpio_low_high_reg &= ~(0b11 << (((my_gpio.pin_num % 8) * 4) + 2));
                *gpio_low_high_reg |= (0b10 << (((my_gpio.pin_num % 8) * 4) + 2));
                my_gpio.gpio->BSRR = (1U << (my_gpio.pin_num + PINS_OF_PORT));
                break;
            default:
                result = ARM_DRIVER_ERROR_PARAMETER;
                break;
        }
    } else {
        result = ARM_GPIO_ERROR_PIN;
    }

    return result;
}

/**
 * @brief Set GPIO event trigger.
 *
 */
static int32_t GPIO_SetEventTrigger(ARM_GPIO_Pin_t pin, ARM_GPIO_EVENT_TRIGGER trigger) {
    int32_t result = ARM_DRIVER_OK;

    if (PIN_IS_AVAILABLE(pin)) {
        GPIO_t my_gpio;

        GPIO_ResetStruct(&my_gpio);
        GPIO_ConvertPin(pin, &my_gpio);

        /* Clear pending bit for safety */
        EXTI->PR |= (1U << my_gpio.pin_num);

        switch (trigger) {
            case ARM_GPIO_TRIGGER_NONE:
                /* Do nothing ! */
                break;
            case ARM_GPIO_TRIGGER_RISING_EDGE:
                EXTI->RTSR |= (1U << my_gpio.pin_num);
                EXTI->FTSR &= ~(1U << my_gpio.pin_num);
                EXTI->IMR |= (1U << my_gpio.pin_num);
                break;
            case ARM_GPIO_TRIGGER_FALLING_EDGE:
                EXTI->FTSR |= (1U << my_gpio.pin_num);
                EXTI->RTSR &= ~(1U << my_gpio.pin_num);
                EXTI->IMR |= (1U << my_gpio.pin_num);
                break;
            case ARM_GPIO_TRIGGER_EITHER_EDGE:
                EXTI->RTSR |= (1U << my_gpio.pin_num);
                EXTI->FTSR |= (1U << my_gpio.pin_num);
                EXTI->IMR |= (1U << my_gpio.pin_num);
                break;
            default:
                /* Fault */
                result = ARM_DRIVER_ERROR_PARAMETER;
                break;
        }
    } else {
        result = ARM_GPIO_ERROR_PIN;
    }

    return result;
}

/**
 * @brief Set GPIO output.
 *
 */
static void GPIO_SetOutput(ARM_GPIO_Pin_t pin, uint32_t val) {
    GPIO_t my_gpio;

    GPIO_ResetStruct(&my_gpio);
    GPIO_ConvertPin(pin, &my_gpio);

    if (PIN_IS_AVAILABLE(pin)) {
        switch (val) {
            case 0:
                my_gpio.gpio->BSRR = (1U << (my_gpio.pin_num + PINS_OF_PORT));
                break;
            case 1:
                my_gpio.gpio->BSRR = (1U << my_gpio.pin_num);
                break;
            default:
                /* fault value */
                break;
        }
    }
}

/**
 * @brief Get GPIO input.
 *
 */
static uint32_t GPIO_GetInput(ARM_GPIO_Pin_t pin) {
    uint32_t val = 0U;
    GPIO_t my_gpio;

    GPIO_ResetStruct(&my_gpio);
    GPIO_ConvertPin(pin, &my_gpio);

    if (PIN_IS_AVAILABLE(pin)) {
        val = (my_gpio.gpio->IDR & (1U << my_gpio.pin_num)) >> my_gpio.pin_num;
    }
    return val;
}

/**
 * @brief GPIO Driver structure.
 *
 */
ARM_DRIVER_GPIO Driver_GPIO0 = {GPIO_Setup,           GPIO_SetDirection,    GPIO_SetOutputMode,
                                GPIO_SetPullResistor, GPIO_SetEventTrigger, GPIO_SetOutput,
                                GPIO_GetInput};

/********************************************************************
 * GPIO Interrupt Service Routine
 ********************************************************************/
/**
 * @brief ISR of external interrupt line 0.
 *
 */
void EXTI0_IRQHandler(void) {
    uint8_t check = (EXTI->PR & (1U << 0));
    uint8_t pin = gpio_exti_active_pin[0];

    if (check) {
        EXTI->PR |= (1U << 0);
        if (gpio_callback_event[0]) {
            if (gpio_exti_active_port[0]->IDR & (1U << 0)) {
                gpio_callback_event[0](pin, ARM_GPIO_TRIGGER_RISING_EDGE);
            } else {
                gpio_callback_event[0](pin, ARM_GPIO_TRIGGER_FALLING_EDGE);
            }
        }
    }
}

/**
 * @brief ISR of external interrupt line 1.
 *
 */
void EXTI1_IRQHandler(void) {
    uint8_t check = (EXTI->PR & (1U << 1));
    uint8_t pin = gpio_exti_active_pin[1];

    if (check) {
        EXTI->PR |= (1U << 1);
        if (gpio_callback_event[1]) {
            if (gpio_exti_active_port[1]->IDR & (1U << 1)) {
                gpio_callback_event[1](pin, ARM_GPIO_TRIGGER_RISING_EDGE);
            } else {
                gpio_callback_event[1](pin, ARM_GPIO_TRIGGER_FALLING_EDGE);
            }
        }
    }
}

/**
 * @brief ISR of external interrupt line 2.
 *
 */
void EXTI2_IRQHandler(void) {
    uint8_t check = (EXTI->PR & (1U << 2));
    uint8_t pin = gpio_exti_active_pin[2];

    if (check) {
        EXTI->PR |= (1U << 2);
        if (gpio_callback_event[2]) {
            if (gpio_exti_active_port[2]->IDR & (1U << 2)) {
                gpio_callback_event[2](pin, ARM_GPIO_TRIGGER_RISING_EDGE);
            } else {
                gpio_callback_event[2](pin, ARM_GPIO_TRIGGER_FALLING_EDGE);
            }
        }
    }
}

/**
 * @brief ISR of external interrupt line 3.
 *
 */
void EXTI3_IRQHandler(void) {
    uint8_t check = (EXTI->PR & (1U << 3));
    uint8_t pin = gpio_exti_active_pin[3];

    if (check) {
        EXTI->PR |= (1U << 3);
        if (gpio_callback_event[3]) {
            if (gpio_exti_active_port[3]->IDR & (1U << 3)) {
                gpio_callback_event[3](pin, ARM_GPIO_TRIGGER_RISING_EDGE);
            } else {
                gpio_callback_event[3](pin, ARM_GPIO_TRIGGER_FALLING_EDGE);
            }
        }
    }
}

/**
 * @brief ISR of external interrupt line 4.
 *
 */
void EXTI4_IRQHandler(void) {
    uint8_t check = (EXTI->PR & (1U << 4));
    uint8_t pin = gpio_exti_active_pin[4];

    if (check) {
        EXTI->PR |= (1U << 4);
        if (gpio_callback_event[4]) {
            if (gpio_exti_active_port[4]->IDR & (1U << 4)) {
                gpio_callback_event[4](pin, ARM_GPIO_TRIGGER_RISING_EDGE);
            } else {
                gpio_callback_event[4](pin, ARM_GPIO_TRIGGER_FALLING_EDGE);
            }
        }
    }
}

/**
 * @brief ISR of external interrupt line 5-9.
 *
 */
void EXTI9_5_IRQHandler(void) {
    uint8_t pin = 0;

    for (int i = 5; i <= 9; i++) {
        if (EXTI->PR & (1U << i)) {
            EXTI->PR |= (1U << i);
            pin = gpio_exti_active_pin[i];
            if (gpio_callback_event[i]) {
                if (gpio_exti_active_port[i]->IDR & (1U << i)) {
                    gpio_callback_event[i](pin, ARM_GPIO_TRIGGER_RISING_EDGE);
                } else {
                    gpio_callback_event[i](pin, ARM_GPIO_TRIGGER_FALLING_EDGE);
                }
            }
        }
    }
}

/**
 * @brief ISR of external interrupt line 10-15.
 *
 */
void EXTI15_10_IRQHandler(void) {
    uint8_t pin = 0;

    for (int i = 10; i <= 15; i++) {
        if (EXTI->PR & (1U << i)) {
            EXTI->PR |= (1U << i);
            pin = gpio_exti_active_pin[i];
            if (gpio_callback_event[i]) {
                if (gpio_exti_active_port[i]->IDR & (1U << i)) {
                    gpio_callback_event[i](pin, ARM_GPIO_TRIGGER_RISING_EDGE);
                } else {
                    gpio_callback_event[i](pin, ARM_GPIO_TRIGGER_FALLING_EDGE);
                }
            }
        }
    }
}
