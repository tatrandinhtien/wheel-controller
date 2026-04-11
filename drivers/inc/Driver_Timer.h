#ifndef DRIVER_TIMER_H_
#define DRIVER_TIMER_H_

#ifdef __cplusplus
extern "C"
{
#endif

/**
 * @brief Tim number.
 */
typedef enum
{
    ARM_TIM_1 = 1,
    ARM_TIM_2,
    ARM_TIM_3,
    ARM_TIM_4
} ARM_TIM_NUM;

/**
 * @brief Channel number.
 */
typedef enum
{
    ARM_CHANNEL_1 = 1,
    ARM_CHANNEL_2,
    ARM_CHANNEL_3,
    ARM_CHANNEL_4
} ARM_TIM_CHANNEL;

/**
 * @brief Encoder count mode.
 */
typedef enum
{
    ARM_UP_EDGE,
    ARM_DOWN_EDGE,
    ARM_EITHER_EDGE
} ARM_ENCODER_MODE;

typedef struct
{
    int32_t (*Setup)   (ARM_TIM_NUM tim, uint16_t freq_hz);
    int32_t (*SetMode) (ARM_TIM_NUM tim, ARM_TIM_CHANNEL channel);
    int32_t (*SetDuty) (ARM_TIM_NUM tim, ARM_TIM_CHANNEL channel, uint8_t duty);
    int32_t (*Trigger) (ARM_TIM_NUM tim, ARM_TIM_CHANNEL channel);
} ARM_DRIVER_TIM_PWM;

typedef struct
{
    int32_t (*Setup) (ARM_TIM_NUM tim);
    int32_t (*SetMode) (ARM_TIM_NUM tim, ARM_ENCODER_MODE mode);
    uint32_t (*GetCount) (ARM_TIM_NUM tim);
    uint8_t (*GetDir) (ARM_TIM_NUM);
} ARM_DRIVER_TIM_ENCODER;

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* DRIVER_TIMER_H_ */