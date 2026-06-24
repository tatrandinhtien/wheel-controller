/**
 * @file    Driver_Timer.c
 * @author  DinhTien (tien.ta.eswe@gmail.com)
 * @brief   Timer Peripheral Driver implementation for PWM output and Encoder interface.
 * @details Implements abstract HAL interfaces for both PWM pulse generation and 
 * quadrature encoder hardware tracking using STM32F103 basic, general, 
 * and advanced timers (TIM1 to TIM4). Handles dynamic register mapping, 
 * preload buffering, hardware input filtering, and safe advanced timer control.
 * @version 1.0
 * @date    2026-05-30
 * * @copyright Copyright (c) 2026
 * */

#include "stm32f103xb.h"

#include "Driver_RCC.h"
#include "Driver_Timer.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/

#define TIM_NUM     4U          /**< Maximum number of standard hardware Timers on STM32F103 (TIM1-TIM4). */
#define ADVANCED    0U          /**< Classification code representing an Advanced Control Timer (TIM1). */
#define GENERAL     1U          /**< Classification code representing a General Purpose Timer (TIM2-TIM4). */

/**
 * @brief  Internal context tracking layout for an active Timer peripheral.
 */
typedef struct {
    TIM_TypeDef* tim;           /**< Pointer to the CMSIS base structure of the register block. */
    uint32_t in_clk;            /**< Input peripheral clock frequency supplying the counter clock. */
    bool type;                  /**< Timer hardware classification category (@ref ADVANCED or @ref GENERAL). */
} TIM_t;

/*******************************************************************************
 * Prototypes
 ******************************************************************************/

static int32_t TIM_Convert(TIM_t* my_tim, uint8_t tim);
static int32_t TIM_PWM_Setup(ARM_TIM_NUM tim, uint16_t freq_hz);
static int32_t TIM_PWM_SetMode(ARM_TIM_NUM tim, ARM_TIM_CHANNEL channel);
static int32_t TIM_PWM_SetDuty(ARM_TIM_NUM tim, ARM_TIM_CHANNEL channel, uint16_t duty);
static int32_t TIM_PWM_Trigger(ARM_TIM_NUM tim, ARM_TIM_CHANNEL channel);
static int32_t TIM_ENCODER_Setup(ARM_TIM_NUM tim);
static int32_t TIM_ENCODER_SetMode(ARM_TIM_NUM tim, ARM_ENCODER_MODE mode);
static int32_t TIM_ENCODER_GetCount(ARM_TIM_NUM tim);
static int32_t TIM_ENCODER_GetDir(ARM_TIM_NUM tim);

/*******************************************************************************
 * Variables
 ******************************************************************************/

TIM_TypeDef* tim_arr[TIM_NUM] = {TIM1, TIM2, TIM3, TIM4}; /**< Index look-up registry table for Timer peripheral bases. */
uint32_t const max_count_val = 65536;                     /**< Maximum capacity limit constant for 16-bit Timer registers. */

/*******************************************************************************
 * Code
 ******************************************************************************/

/**
 * @brief  Converts a numerical driver enumeration into internal layout contextual data.
 * @param  tim: Absolute numeric ID index of the timer (1 to 4).
 * @param  my_tim: Workspace reference structure pointer to fill with resolved metadata.
 * @retval ARM_DRIVER_OK: Look-up operation completed successfully.
 * @retval ARM_DRIVER_ERROR: Timer index out of bounds or pointer is null.
 */
static int32_t TIM_Convert(TIM_t* my_tim, uint8_t tim) {
    int32_t result = ARM_DRIVER_ERROR;

    if (my_tim != NULL && tim > 0 && tim < 5) {
        my_tim->tim = tim_arr[tim - 1];
        my_tim->in_clk = SystemCoreClock;
        if (tim == 1) {
            my_tim->type = ADVANCED;
        } else {
            my_tim->type = GENERAL;
        }
        result = ARM_DRIVER_OK;
    }

    return result;
}

/**
 * @brief  Configures the overall base output frequency for PWM operations by adjusting PSC and ARR.
 * @param  tim: Chosen timer peripheral index target (@ref ARM_TIM_NUM).
 * @param  freq_hz: Desired cycle base execution frequency rate in Hertz.
 * @retval ARM_DRIVER_OK: Calculation logic successfully assigned to register blocks.
 * @retval ARM_DRIVER_ERROR: Erroneous parameter frequency or conversion initialization failure.
 */
static int32_t TIM_PWM_Setup(ARM_TIM_NUM tim, uint16_t freq_hz) {
    int32_t result = ARM_DRIVER_OK;
    uint32_t total_divider, psc_calc = 0;
    TIM_t my_tim;

    if (TIM_Convert(&my_tim, tim) == ARM_DRIVER_ERROR) {
        result = ARM_DRIVER_ERROR;
        return result;
    }

    if (freq_hz == 0 || freq_hz > SystemCoreClock) {
        return ARM_DRIVER_ERROR;
    }

    total_divider = SystemCoreClock / freq_hz;
    switch (tim) {
        case ARM_TIM_1:
            RCC_TIM1_CLK_EN();
            break;
        case ARM_TIM_2:
            RCC_TIM2_CLK_EN();
            break;
        case ARM_TIM_3:
            RCC_TIM3_CLK_EN();
            break;
        case ARM_TIM_4:
            RCC_TIM4_CLK_EN();
            break;
        default:
            /* Can not reach here */
            break;
    }

    /* Calculate the prescaler and auto-reload values optimally for 16-bit register window bounds */
    psc_calc = (total_divider - 1) / 65536;
    my_tim.tim->PSC = (uint16_t)psc_calc;
    my_tim.tim->ARR = (uint16_t)((total_divider / (psc_calc + 1)) - 1);

    return result;
}

/**
 * @brief  Configures a specific timer channel to work in PWM Mode 1 with Output Preload enabled.
 * @param  tim: Chosen timer peripheral index target (@ref ARM_TIM_NUM).
 * @param  channel: Channel hardware pathway target layout indicator (@ref ARM_TIM_CHANNEL).
 * @retval ARM_DRIVER_OK: Channel operational layout bits committed successfully.
 * @retval ARM_DRIVER_ERROR: Faulty structural configuration index parameters passed.
 */
static int32_t TIM_PWM_SetMode(ARM_TIM_NUM tim, ARM_TIM_CHANNEL channel) {
    int32_t result = ARM_DRIVER_OK;
    TIM_t my_tim;

    if (TIM_Convert(&my_tim, tim) == ARM_DRIVER_ERROR) {
        result = ARM_DRIVER_ERROR;
        return result;
    }

    switch (channel) {
        case ARM_CHANNEL_1:
            /* Channel 1 output selection */
            my_tim.tim->CCMR1 &= ~(0b11 << TIM_CCMR1_CC1S_Pos);
            /* Configure PWM Mode 1 (Output High when CNT < CCR1, otherwise Low) */
            my_tim.tim->CCMR1 &= ~(0b111 << TIM_CCMR1_OC1M_Pos);
            my_tim.tim->CCMR1 |= (0b110 << TIM_CCMR1_OC1M_Pos);
            /* Enable Output Compare 1 Preload Buffer */
            my_tim.tim->CCMR1 |= (1 << TIM_CCMR1_OC1PE_Pos);
            break;
        case ARM_CHANNEL_2:
            my_tim.tim->CCMR1 &= ~(0b11 << TIM_CCMR1_CC2S_Pos);
            my_tim.tim->CCMR1 &= ~(0b111 << TIM_CCMR1_OC2M_Pos);
            my_tim.tim->CCMR1 |= (0b110 << TIM_CCMR1_OC2M_Pos);
            my_tim.tim->CCMR1 |= (1 << TIM_CCMR1_OC2PE_Pos);
            break;
        case ARM_CHANNEL_3:
            my_tim.tim->CCMR2 &= ~(0b11 << TIM_CCMR2_CC3S_Pos);
            my_tim.tim->CCMR2 &= ~(0b111 << TIM_CCMR2_OC3M_Pos);
            my_tim.tim->CCMR2 |= (0b110 << TIM_CCMR2_OC3M_Pos);
            my_tim.tim->CCMR2 |= (1 << TIM_CCMR2_OC3PE_Pos);
            break;
        case ARM_CHANNEL_4:
            my_tim.tim->CCMR2 &= ~(0b11 << TIM_CCMR2_CC4S_Pos);
            my_tim.tim->CCMR2 &= ~(0b111 << TIM_CCMR2_OC4M_Pos);
            my_tim.tim->CCMR2 |= (0b110 << TIM_CCMR2_OC4M_Pos);
            my_tim.tim->CCMR2 |= (1 << TIM_CCMR2_OC4PE_Pos);
            break;
        default:
            result = ARM_DRIVER_ERROR;
            break;
    }

    return result;
}

/**
 * @brief  Updates the comparative duty cycle scaling value for output generation.
 * @param  tim: Chosen timer peripheral index target (@ref ARM_TIM_NUM).
 * @param  channel: Channel hardware pathway target layout indicator (@ref ARM_TIM_CHANNEL).
 * @param  duty: Resolution parameter scale bound from 0 to 10000 (representing 0.00% to 100.00%).
 * @retval ARM_DRIVER_OK: Duty registration written down into active CCR block.
 * @retval ARM_DRIVER_ERROR: Invalid base configurations detected.
 */
static int32_t TIM_PWM_SetDuty(ARM_TIM_NUM tim, ARM_TIM_CHANNEL channel, uint16_t duty) {
    int32_t result = ARM_DRIVER_OK;
    uint32_t load_val;
    TIM_t my_tim;

    if (TIM_Convert(&my_tim, tim) == ARM_DRIVER_ERROR) {
        result = ARM_DRIVER_ERROR;
        return result;
    }

    /* Enforce maximum threshold ceiling saturation */
    if (duty > 10000) {
        duty = 10000;
    }

    /* Transform percentage factor into proportional absolute register clock ticking value */
    load_val = (uint32_t)(((uint32_t)duty * (my_tim.tim->ARR + 1)) / 10000);

    switch (channel) {
        case ARM_CHANNEL_1:
            my_tim.tim->CCR1 = load_val;
            break;
        case ARM_CHANNEL_2:
            my_tim.tim->CCR2 = load_val;
            break;
        case ARM_CHANNEL_3:
            my_tim.tim->CCR3 = load_val;
            break;
        case ARM_CHANNEL_4:
            my_tim.tim->CCR4 = load_val;
            break;
        default:
            result = ARM_DRIVER_ERROR;
            break;
    }

    return result;
}

/**
 * @brief  Enables channel output comparison toggles and starts the master counter engine clock.
 * @note   Automatically sets the Main Output Enable (MOE) safety bridge if handling an Advanced Timer.
 * @param  tim: Chosen timer peripheral index target (@ref ARM_TIM_NUM).
 * @param  channel: Channel hardware pathway target layout indicator (@ref ARM_TIM_CHANNEL).
 * @retval ARM_DRIVER_OK: Clock counter core launched into dynamic mode.
 * @retval ARM_DRIVER_ERROR: Structural conversion error.
 */
static int32_t TIM_PWM_Trigger(ARM_TIM_NUM tim, ARM_TIM_CHANNEL channel) {
    int32_t result = ARM_DRIVER_OK;
    TIM_t my_tim;

    if (TIM_Convert(&my_tim, tim) == ARM_DRIVER_ERROR) {
        result = ARM_DRIVER_ERROR;
        return result;
    }

    switch (channel) {
        case ARM_CHANNEL_1:
            my_tim.tim->CCER |= (1 << TIM_CCER_CC1E_Pos);
            break;
        case ARM_CHANNEL_2:
            my_tim.tim->CCER |= (1 << TIM_CCER_CC2E_Pos);
            break;
        case ARM_CHANNEL_3:
            my_tim.tim->CCER |= (1 << TIM_CCER_CC3E_Pos);
            break;
        case ARM_CHANNEL_4:
            my_tim.tim->CCER |= (1 << TIM_CCER_CC4E_Pos);
            break;
        default:
            result = ARM_DRIVER_ERROR;
            return result;
    }

    /* Advanced Control Timers (TIM1) require break-deadtime master output activation */
    if (my_tim.type == ADVANCED) {
        my_tim.tim->BDTR |= (1 << TIM_BDTR_MOE_Pos);
    }
    
    /* Enable counter (CEN) */
    my_tim.tim->CR1 |= (1 << TIM_CR1_CEN_Pos);

    return result;
}

/**
 * @brief Global structure instance for binding Timer PWM Driver capabilities.
 */
ARM_DRIVER_TIM_PWM Driver_TIM0 = {TIM_PWM_Setup, TIM_PWM_SetMode, TIM_PWM_SetDuty, TIM_PWM_Trigger};

/**
 * @brief  Configures Channel 1 & 2 inputs into Quadrature Encoder interface mode with debounce filtering.
 * @param  tim: Chosen timer peripheral index target (@ref ARM_TIM_NUM).
 * @retval ARM_DRIVER_OK: Quadrature capture paths initialized cleanly.
 * @retval ARM_DRIVER_ERROR: Error mapping registry.
 */
static int32_t TIM_ENCODER_Setup(ARM_TIM_NUM tim) {
    int32_t result = ARM_DRIVER_OK;
    TIM_t my_tim;

    if (TIM_Convert(&my_tim, tim) == ARM_DRIVER_ERROR) {
        result = ARM_DRIVER_ERROR;
        return result;
    }

    switch (tim) {
        case ARM_TIM_1:
            RCC_TIM1_CLK_EN();
            break;
        case ARM_TIM_2:
            RCC_TIM2_CLK_EN();
            break;
        case ARM_TIM_3:
            RCC_TIM3_CLK_EN();
            break;
        case ARM_TIM_4:
            RCC_TIM4_CLK_EN();
            break;
        default:
            /* Can not reach here */
            break;
    }

    /* Preload maximum overhead boundary value into 16-bit Auto-Reload Register */
    my_tim.tim->ARR = 0xFFFF;

    /* Configure Channel 1 and 2 to map Input Capture targets straight onto TI1 and TI2 inputs */
    my_tim.tim->CCMR1 &= ~(0b11 << TIM_CCMR1_CC1S_Pos);
    my_tim.tim->CCMR1 |= (0b01 << TIM_CCMR1_CC1S_Pos);
    my_tim.tim->CCMR1 &= ~(0b11 << TIM_CCMR1_CC2S_Pos);
    my_tim.tim->CCMR1 |= (0b01 << TIM_CCMR1_CC2S_Pos);

    /* Hardware Input Filtering: Set filter value to sample each 8 internal pulses to suppress glitch noise */
    my_tim.tim->CCMR1 &= ~(0b1111 << TIM_CCMR1_IC1F_Pos);
    my_tim.tim->CCMR1 |= (0b0011 << TIM_CCMR1_IC1F_Pos);
    my_tim.tim->CCMR1 &= ~(0b1111 << TIM_CCMR1_IC2F_Pos);
    my_tim.tim->CCMR1 |= (0b0011 << TIM_CCMR1_IC2F_Pos);

    /* Disable capture input prescalers to process every valid transitional clock event immediately */
    my_tim.tim->CCMR1 &= ~(0b11 << TIM_CCMR1_IC1PSC_Pos);
    my_tim.tim->CCMR1 &= ~(0b11 << TIM_CCMR1_IC2PSC_Pos);

    /* Set capture inputs polarity to non-inverted (rising edge transition tracking) */
    my_tim.tim->CCER &= ~(1 << TIM_CCER_CC1P_Pos);
    my_tim.tim->CCER &= ~(1 << TIM_CCER_CC2P_Pos);

    return result;
}

/**
 * @brief  Selects the explicit encoder tracking edge mode scheme and boots up counting core.
 * @param  tim: Chosen timer peripheral index target (@ref ARM_TIM_NUM).
 * @param  mode: Counting scheme profile setting type (@ref ARM_ENCODER_MODE).
 * @retval ARM_DRIVER_OK: Clock counter interface engaged active.
 * @retval ARM_DRIVER_ERROR_PARAMETER: Triggered unsupported tracking edge profile mode.
 */
static int32_t TIM_ENCODER_SetMode(ARM_TIM_NUM tim, ARM_ENCODER_MODE mode) {
    int32_t result = ARM_DRIVER_OK;
    TIM_t my_tim;

    if (TIM_Convert(&my_tim, tim) == ARM_DRIVER_ERROR) {
        result = ARM_DRIVER_ERROR;
        return result;
    }

    switch (mode) {
        case ARM_UP_EDGE:
            /* Hardware limitation or driver implementation scope restriction */
            result = ARM_DRIVER_ERROR_PARAMETER;
            break;
        case ARM_DOWN_EDGE:
            /* Hardware limitation or driver implementation scope restriction */
            result = ARM_DRIVER_ERROR_PARAMETER;
            break;
        case ARM_EITHER_EDGE:
            /* Encoder Mode 3: Counter tracks relative phases synchronously on both TI1 and TI2 transitions (X4 Mode) */
            my_tim.tim->SMCR &= ~(0b111 << TIM_SMCR_SMS_Pos);
            my_tim.tim->SMCR |= (0b011 << TIM_SMCR_SMS_Pos);
            break;
        default:
            result = ARM_DRIVER_ERROR_PARAMETER;
            break;
    }

    /* Initialize reference tracker register point back to 0 baseline */
    my_tim.tim->CNT = 0;

    /* Turn on master counter engine clock */
    my_tim.tim->CR1 |= (1 << TIM_CR1_CEN_Pos);

    return result;
}

/**
 * @brief  Fetches the active raw rotational ticking feedback integer step from register blocks.
 * @param  tim: Chosen timer peripheral index target (@ref ARM_TIM_NUM).
 * @return int32_t: Returns current absolute ticks value, or -1 if lookup structure validation fails.
 */
static int32_t TIM_ENCODER_GetCount(ARM_TIM_NUM tim) {
    TIM_t my_tim;

    if (TIM_Convert(&my_tim, tim) == ARM_DRIVER_ERROR) {
        return -1;
    }

    return (int32_t)my_tim.tim->CNT;
}

/**
 * @brief  Inspects register status bits to detect current hardware wheel rotation direction status.
 * @param  tim: Chosen timer peripheral index target (@ref ARM_TIM_NUM).
 * @return int32_t: Returns 0 for upcounting (Forward), 1 for downcounting (Reverse), or -1 on failure.
 */
static int32_t TIM_ENCODER_GetDir(ARM_TIM_NUM tim) {
    uint8_t dir;
    TIM_t my_tim;
    
    if (TIM_Convert(&my_tim, tim) == ARM_DRIVER_ERROR) {
        return -1;
    }
    
    /* Read hardware Direction (DIR) status bit from Control Register 1 */
    dir = (uint8_t)((my_tim.tim->CR1 & (1 << TIM_CR1_DIR_Pos)) >> TIM_CR1_DIR_Pos);

    return (int32_t)dir;
}

/**
 * @brief Global structure instance for binding Timer Encoder Driver capabilities.
 */
ARM_DRIVER_TIM_ENCODER Driver_TIM1 = {TIM_ENCODER_Setup, TIM_ENCODER_SetMode, TIM_ENCODER_GetCount, TIM_ENCODER_GetDir};
