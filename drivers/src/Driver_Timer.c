#include "stm32f103xb.h"

#include "Driver_Timer.h"
#include "Driver_RCC.h"

#define TIM_NUM     4U

#define ADVANCED    0U
#define GENERAL     1U

TIM_TypeDef *tim_arr[TIM_NUM] = {TIM1, TIM2, TIM3, TIM4};

uint32_t const max_count_val = 65536;

typedef struct
{
    TIM_TypeDef *tim;
    uint32_t in_clk;    /* Clock input */
    bool type;
} TIM_t;

/**
 * @brief Convert from tim num to get needed parameters.
 */
static void TIM_Convert(TIM_t *my_tim, uint8_t tim)
{
    if (my_tim != NULL)
    {
        my_tim->tim = tim_arr[tim-1];
        my_tim->in_clk = SystemCoreClock;
        if (tim == 1)
        {
            my_tim->type = ADVANCED;
        }
        else
        {
            my_tim->type = GENERAL;
        }
    }
}

/**
 * @brief Set up frequency of PWM by configuring PSC and ARR register.
 */
static int32_t TIM_PWM_Setup (ARM_TIM_NUM tim, uint16_t freq_hz)
{
    int32_t result = ARM_DRIVER_OK;
    uint32_t psc_calc = 0;
    uint32_t arr_calc = 0;

    TIM_t my_tim;
    TIM_Convert(&my_tim, tim);

    if (freq_hz == 0 || freq_hz > SystemCoreClock)
    {
        return ARM_DRIVER_ERROR;
    }

    /* Calculate the prescaler and reload value */
    psc_calc = SystemCoreClock / (freq_hz * max_count_val);
    arr_calc = (SystemCoreClock / (((psc_calc + 1) * freq_hz) - 1));

    my_tim.tim->PSC = (uint16_t)psc_calc;
    my_tim.tim->ARR = (uint16_t)arr_calc;

    return result;
}

/**
 * @brief Set mode for PWM, enable the Preload.
 */
static int32_t TIM_PWM_SetMode (ARM_TIM_NUM tim, ARM_TIM_CHANNEL channel)
{
    int32_t result = ARM_DRIVER_OK;
    TIM_t my_tim;
    TIM_Convert(&my_tim, tim);

    switch(channel)
    {
        case ARM_CHANNEL_1:
            /* Channel 1 ouput */
            my_tim.tim->CCMR1 &= ~(0b11 << TIM_CCMR1_CC1S_Pos);
            /* PWM mode 1 */
            my_tim.tim->CCMR1 &= ~(0b111 << TIM_CCMR1_OC1M_Pos);
            my_tim.tim->CCMR1 |= (0b110 << TIM_CCMR1_OC1M_Pos);
            /* Enable Preload */
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
    }

    return result;
}

/**
 * @brief Set duty cycle for PWM.
 */
static int32_t TIM_PWM_SetDuty (ARM_TIM_NUM tim, ARM_TIM_CHANNEL channel, uint8_t duty)
{
    int32_t result = ARM_DRIVER_OK;
    TIM_t my_tim;
    TIM_Convert(&my_tim, tim);

    uint32_t load_val = ((uint32_t) duty * my_tim.tim->ARR) / 100;

    switch (channel)
    {
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
 * @brief Trigger and enbale the safety for advanced tim (TIM1).
 */
static int32_t TIM_PWM_Trigger (ARM_TIM_NUM tim, ARM_TIM_CHANNEL channel)
{
    int32_t result = ARM_DRIVER_OK;

    TIM_t my_tim;
    TIM_Convert(&my_tim, tim);

    switch(channel)
    {
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
            break;
    }

    if (my_tim.type == ADVANCED)
    {
        my_tim.tim->BDTR |= (1 << TIM_BDTR_MOE_Pos);
    }
    my_tim.tim->CR1 |= (1 << TIM_CR1_CEN_Pos);

    return result;
}

ARM_DRIVER_TIM_PWM Driver_TIM0 = {
    TIM_PWM_Setup,
    TIM_PWM_SetMode,
    TIM_PWM_SetDuty,
    TIM_PWM_Trigger
};


/**
 * @brief Config channel 1 and 2, enable filter each 8 pulses, disable input capture and non inverter.
 */
static int32_t TIM_ENCODER_Setup (ARM_TIM_NUM tim)
{
    int32_t result = ARM_DRIVER_OK;

    TIM_t my_tim;
    TIM_Convert(&my_tim, tim);

    /* Load value into ARR reg*/
    my_tim.tim->ARR = 0xFFFF;

    /* Config channel 1 and 2 */
    my_tim.tim->CCMR1 &= ~(0b11 << TIM_CCMR1_CC1S_Pos);
    my_tim.tim->CCMR1 |= (0b01 << TIM_CCMR1_CC1S_Pos);
    my_tim.tim->CCMR1 &= ~(0b11 << TIM_CCMR1_CC2S_Pos);
    my_tim.tim->CCMR1 |= (0b01 << TIM_CCMR1_CC2S_Pos);

    /* Filter 8  */
    my_tim.tim->CCMR1 &= ~(0b1111 << TIM_CCMR1_IC1F_Pos);
    my_tim.tim->CCMR1 |= (0b0011 << TIM_CCMR1_IC1F_Pos);
    my_tim.tim->CCMR1 &= ~(0b1111 << TIM_CCMR1_IC2F_Pos);
    my_tim.tim->CCMR1 |= (0b0011 << TIM_CCMR1_IC2F_Pos);

    /* Disable input capture psc */
    my_tim.tim->CCMR1 &= ~(0b11 << TIM_CCMR1_IC1PSC_Pos);
    my_tim.tim->CCMR1 &= ~(0b11 << TIM_CCMR1_IC2PSC_Pos);

    /* Non inverter */
    my_tim.tim->CCER &= ~(1 << TIM_CCER_CC1P_Pos);
    my_tim.tim->CCER &= ~(1 << TIM_CCER_CC2P_Pos);

    return result;
}

/**
 * @brief Choose mode for encoder, enbale count.
 */
static int32_t TIM_ENCODER_SetMode (ARM_TIM_NUM tim, ARM_ENCODER_MODE mode)
{
    int32_t result = ARM_DRIVER_OK;

    TIM_t my_tim;
    TIM_Convert(&my_tim, tim);

    switch (mode)
    {
        case ARM_UP_EDGE:
            /* Does not support yet */
            result = ARM_DRIVER_ERROR_PARAMETER;
            break;
        case ARM_DOWN_EDGE:
            /* Does not support yet */
            result = ARM_DRIVER_ERROR_PARAMETER;
             break;
        case ARM_EITHER_EDGE:
            my_tim.tim->SMCR &= ~(0b111 << TIM_SMCR_SMS_Pos);
            my_tim.tim->SMCR |= (0b011 << TIM_SMCR_SMS_Pos);
            break;
        default:
            /* Does not come here */
            result = ARM_DRIVER_ERROR_PARAMETER;
            break;
    }

    /* Reset count to 0 */
    my_tim.tim->CNT = 0;

    /* Enable count */
    my_tim.tim->CR1 |= (1 << TIM_CR1_CEN_Pos);

    return result;
}

/**
 * @brief Get count from encoder.
 **/
static uint32_t TIM_ENCODER_GetCount (ARM_TIM_NUM tim)
{
    TIM_t my_tim;
    TIM_Convert(&my_tim, tim);

    return my_tim.tim->CNT;
}

/**
 * @brief Get direction from encoder.
 **/
static uint8_t TIM_ENCODER_GetDirection (ARM_TIM_NUM tim)
{
    TIM_t my_tim;
    TIM_Convert(&my_tim, tim);
    uint8_t dir = ((my_tim.tim->CR1 & (1 << TIM_CR1_DIR_Pos)) >> TIM_CR1_DIR_Pos);

    return dir;
}

ARM_DRIVER_TIM_ENCODER Driver_TIM1 = {
    TIM_ENCODER_Setup,
    TIM_ENCODER_SetMode,
    TIM_ENCODER_GetCount,
    TIM_ENCODER_GetDirection
};