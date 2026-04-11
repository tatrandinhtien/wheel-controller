#include "test_timer.h"
#include "test_config.h"

#include "system_stm32f1xx.h"

#include "Driver_RCC.h"
#include "Driver_GPIO.h"
#include "Driver_Timer.h"

/**
 * @brief Software delay
 */
static void delay(uint32_t time)
{
    while(time--)
    {
        __asm("nop");
    }
}

#ifdef TIMER_TEST

extern ARM_DRIVER_GPIO Driver_GPIO0;
extern ARM_DRIVER_RCC Driver_RCC0;
extern ARM_DRIVER_TIM_PWM Driver_TIM0;
extern ARM_DRIVER_TIM_ENCODER Driver_TIM1;


void test_control_motor(void)
{
    /* Clock enable */
    RCC_TIM1_CLK_EN();
    RCC_GPIOA_CLK_EN();
    RCC_GPIOB_CLK_EN();

    /* MOTOR IN1 config */
    Driver_GPIO0.Setup(MOTOR_IN1, NULL);
    Driver_GPIO0.SetDirection(MOTOR_IN1, ARM_GPIO_OUTPUT);
    Driver_GPIO0.SetOutputMode(MOTOR_IN1, ARM_GPIO_PUSH_PULL);
    Driver_GPIO0.SetOutput(MOTOR_IN1, HIGH);

    /* MOTOR IN2 config */
    Driver_GPIO0.Setup(MOTOR_IN2, NULL);
    Driver_GPIO0.SetDirection(MOTOR_IN2, ARM_GPIO_OUTPUT);
    Driver_GPIO0.SetOutputMode(MOTOR_IN2, ARM_GPIO_PUSH_PULL);
    Driver_GPIO0.SetOutput(MOTOR_IN2, LOW);

    /* MOTOR PWM config */
    Driver_GPIO0.Setup(MOTOR_PWM_PIN, NULL);
    Driver_GPIO0.SetDirection(MOTOR_PWM_PIN, ARM_GPIO_AF_OUTPUT);
    Driver_GPIO0.SetOutputMode(MOTOR_PWM_PIN, ARM_AFIO_PUSH_PULL);

    /* TIM1 CHANNEL 1, PWM config */
    Driver_TIM0.Setup(ARM_TIM_1, PWM_FREQ);
    Driver_TIM0.SetMode(ARM_TIM_1, ARM_CHANNEL_1);
    Driver_TIM0.SetDuty(ARM_TIM_1, ARM_CHANNEL_1, 50);
    Driver_TIM0.Trigger(ARM_TIM_1, ARM_CHANNEL_1);

    int32_t count = 0;
    int32_t step  = 5;

    while (1)
    {
        Driver_TIM0.SetDuty(ARM_TIM_1, ARM_CHANNEL_1, count);
        count += step;

        if (count <= 0 || count >= 100)
        {
            step = -step;
        }

        delay(DELAY_TIME/100);
    }
}

void test_read_encoder(void)
{

}

void test_timer_run(void)
{
    Driver_RCC0.SetSystemClock();
    SystemCoreClockUpdate();

    #ifdef CONTROL_MOTOR_TEST
    test_control_motor();
    #endif

    #ifdef READ_ENCODER_TEST
    test_read_encoder();
    #endif

}

#endif /* TIMER_TEST */