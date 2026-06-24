/**
 * @file    Driver_GPIO.c
 * @author  DinhTien (tien.ta.eswe@gmail.com)
 * @brief   GPIO Peripheral Driver implementation following CMSIS-Driver standard.
 * @details This driver manages GPIO port/pin mapping, input/output configurations,
 * internal pull-up/pull-down resistors, and external interrupt (EXTI) 
 * handling with dynamic callback registration for STM32F103 MCU.
 * @version 1.0
 * @date    2026-04-04
 * * @copyright Copyright (c) 2026
 * */

#include "Driver_GPIO.h"
#include "Driver_RCC.h"
#include "stm32f103xb.h"

/********************************************************************
 * Definitions
 ********************************************************************/

#define GPIO_MAX_PINS 48U                           /**< Maximum number of pins supported across standard ports (A, B, C). */
#define PIN_IS_AVAILABLE(n) ((n) < GPIO_MAX_PINS)   /**< Macro to validate if a pin number is within acceptable range. */
#define GPIO_NUMS 3U                                /**< Total number of handled GPIO ports (GPIOA to GPIOC). */
#define PINS_OF_PORT 16U                            /**< Number of pins per individual GPIO port. */

/**
 * @brief  Internal structure representing an unpacked GPIO pin configuration.
 */
typedef struct {
    GPIO_TypeDef* gpio;     /**< Pointer to the CMSIS peripheral register base (e.g., GPIOA). */
    uint8_t port_num;       /**< Numeric index representing the port (0=A, 1=B, 2=C). */
    uint8_t pin_num;        /**< Bit position index representing the specific pin (0 to 15). */
} GPIO_t;

/*******************************************************************************
 * Prototypes
 ******************************************************************************/

static void GPIO_ResetStruct(GPIO_t* my_gpio);
static void GPIO_ConvertPin(ARM_GPIO_Pin_t pin, GPIO_t* my_gpio);
static int32_t GPIO_Setup(ARM_GPIO_Pin_t pin, ARM_GPIO_SignalEvent_t cb_event);
static int32_t GPIO_SetDirection(ARM_GPIO_Pin_t pin, ARM_GPIO_DIRECTION direction);
static int32_t GPIO_SetOutputMode(ARM_GPIO_Pin_t pin, ARM_GPIO_OUTPUT_MODE mode);
static int32_t GPIO_SetPullResistor(ARM_GPIO_Pin_t pin, ARM_GPIO_PULL_RESISTOR resistor);
static int32_t GPIO_SetEventTrigger(ARM_GPIO_Pin_t pin, ARM_GPIO_EVENT_TRIGGER trigger);
static void GPIO_SetOutput(ARM_GPIO_Pin_t pin, uint32_t val);
static uint32_t GPIO_GetInput(ARM_GPIO_Pin_t pin);

/*******************************************************************************
 * Variables
 ******************************************************************************/

static GPIO_TypeDef* gpio_arr[GPIO_NUMS] = {GPIOA, GPIOB, GPIOC};   /**< Lookup array for port base addresses. */
static ARM_GPIO_SignalEvent_t gpio_callback_event[PINS_OF_PORT];    /**< Array of registered function pointers for callbacks. */
static uint8_t gpio_exti_active_pin[PINS_OF_PORT];                  /**< Mapping from EXTI line index back to raw pin number. */
static GPIO_TypeDef* gpio_exti_active_port[PINS_OF_PORT];           /**< Reference to active port mapping for interrupt verification. */

/********************************************************************
 * Code
 ********************************************************************/

/**
 * @brief  Resets the members of the internal GPIO_t structure to safe default values.
 * @param  my_gpio: Pointer to the GPIO workspace structure to be cleared.
 * @return None
 */
static void GPIO_ResetStruct(GPIO_t* my_gpio) {
    my_gpio->gpio = NULL;
    my_gpio->port_num = 0;
    my_gpio->pin_num = 0;
}

/**
 * @brief  Converts a linear CMSIS-style pin index into discrete port, pin, and register base references.
 * @param  pin: The linear absolute pin number (e.g., pin 16 maps to Port B, pin 0).
 * @param  my_gpio: Pointer to the destination GPIO structure to hold the parsed results.
 * @return None
 */
static void GPIO_ConvertPin(ARM_GPIO_Pin_t pin, GPIO_t* my_gpio) {
    if (my_gpio != NULL) {
        my_gpio->port_num = (uint8_t)pin / PINS_OF_PORT;
        my_gpio->gpio = gpio_arr[my_gpio->port_num];
        my_gpio->pin_num = (uint8_t)pin % PINS_OF_PORT;
    }
}

/**
 * @brief  Initializes and enables clock routing for the target GPIO port, and optionally configures EXTI.
 * @param  pin: Absolute pin number to be initialized.
 * @param  cb_event: Callback function handler to attach to the external interrupt line. Pass NULL if not using interrupts.
 * @retval ARM_DRIVER_OK: Initialization completed successfully.
 * @retval ARM_GPIO_ERROR_PIN: Given pin parameter is out of bounds or invalid.
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
 * @brief  Configures the direction mode (Input, Output, or Alternate Function Output) of a specific pin.
 * @note   Applies configuration changes straight to STM32 CRL/CRH registers with fixed 2MHz slew rate for outputs.
 * @param  pin: Absolute pin number to be configured.
 * @param  direction: Target direction enum value (@ref ARM_GPIO_DIRECTION).
 * @retval ARM_DRIVER_OK: Configuration written successfully.
 * @retval ARM_DRIVER_ERROR_PARAMETER: Invalid direction flag passed.
 * @retval ARM_GPIO_ERROR_PIN: Specified pin index is invalid.
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
 * @brief  Sets the output driver hardware configuration (Push-Pull, Open-Drain) for standard or AFIO usage.
 * @param  pin: Absolute pin number to adjust.
 * @param  mode: Selected electrical output behavior enum (@ref ARM_GPIO_OUTPUT_MODE).
 * @retval ARM_DRIVER_OK: Configuration written successfully.
 * @retval ARM_DRIVER_ERROR_PARAMETER: Invalid mode flag passed.
 * @retval ARM_GPIO_ERROR_PIN: Specified pin index is out of bounds.
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
 * @brief  Enables, disables, or configures the internal Pull-up/Pull-down resistor network for an input pin.
 * @note   Leverages STM32F1 BSRR / ODR trick to transition between pull-up and pull-down states.
 * @param  pin: Absolute pin number to configure.
 * @param  resistor: Desired configuration structure enum (@ref ARM_GPIO_PULL_RESISTOR).
 * @retval ARM_DRIVER_OK: Pull resistors altered successfully.
 * @retval ARM_DRIVER_ERROR_PARAMETER: Invalid resistor setting argument.
 * @retval ARM_GPIO_ERROR_PIN: Specified pin index is invalid.
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
 * @brief  Sets up the EXTI peripheral line triggers (Rising, Falling, or Edge transitions).
 * @param  pin: Absolute pin number acting as the external trigger line source.
 * @param  trigger: Chosen condition flag to fire interrupts (@ref ARM_GPIO_EVENT_TRIGGER).
 * @retval ARM_DRIVER_OK: Edge detection flags altered successfully.
 * @retval ARM_DRIVER_ERROR_PARAMETER: Invalid edge classification argument.
 * @retval ARM_GPIO_ERROR_PIN: Specified pin index is out of bounds.
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
 * @brief  Sets an output pin state to high logic (1) or low logic (0) using atomic bit set/reset operations.
 * @param  pin: Absolute pin number to toggle.
 * @param  val: Desired bit status (0 = Logic LOW, 1 = Logic HIGH).
 * @return None
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
 * @brief  Reads the physical binary state from the target peripheral's Input Data Register (IDR).
 * @param  pin: Absolute pin number to poll.
 * @return uint32_t: Returns 1 if the input pin is logic HIGH, or 0 if it is logic LOW.
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
 * @brief Global structure instance for interface capabilities binding.
 */
ARM_DRIVER_GPIO Driver_GPIO0 = {GPIO_Setup,           GPIO_SetDirection,    GPIO_SetOutputMode,
                                GPIO_SetPullResistor, GPIO_SetEventTrigger, GPIO_SetOutput,
                                GPIO_GetInput};

/**
 * @brief  Interrupt Service Routine (ISR) serving EXTI line 0 transitions.
 * @return None
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
 * @brief  Interrupt Service Routine (ISR) serving EXTI line 1 transitions.
 * @return None
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
 * @brief  Interrupt Service Routine (ISR) serving EXTI line 2 transitions.
 * @return None
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
 * @brief  Interrupt Service Routine (ISR) serving EXTI line 3 transitions.
 * @return None
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
 * @brief  Interrupt Service Routine (ISR) serving EXTI line 4 transitions.
 * @return None
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
 * @brief  Shared Interrupt Service Routine (ISR) serving EXTI multiplexed lines 5 to 9.
 * @return None
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
 * @brief  Shared Interrupt Service Routine (ISR) serving EXTI multiplexed lines 10 to 15.
 * @return None
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
