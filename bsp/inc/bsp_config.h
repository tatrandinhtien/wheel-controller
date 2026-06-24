/**
 * @file    bsp_config.h
 * @author  DinhTien (tien.ta.eswe@gmail.com)
 * @brief   Hardware pin mapping and global system configurations.
 * @details Centralizes all physical pin definitions, peripheral allocations, 
 * and configuration constants (PWM frequencies, Baudrates) to decouple 
 * hardware dependencies from upper application layers.
 * @version 1.0
 * @date    2026-06-23
 *
 * @copyright Copyright (c) 2026
 *
 */

#ifndef BSP_CONFIG_H_
#define BSP_CONFIG_H_

#ifdef __cplusplus
extern "C" {
#endif

/*******************************************************************************
 * Definitions
 ******************************************************************************/

/* Global Logic State Constants */
#define ENABLE              1               /**< Generic enable condition utility state. */
#define DISABLE             0               /**< Generic disable condition utility state. */

#define HIGH                1               /**< Logic high voltage level signal. */
#define LOW                 0               /**< Logic low voltage level signal. */

#define ON                  0               /**< Active-low component power state ON. */
#define OFF                 1               /**< Active-low component power state OFF. */

#define DELAY_TIME          5000000         /**< Software polling loop timeout dummy limit counter. */

/* Physical GPIO Absolute Pin Index Definitions (Linear Hardware Mapping) */
#define PORTA_PIN0          0U              /**< Port A - Pin 0 index. */
#define PORTA_PIN1          1U              /**< Port A - Pin 1 index. */
#define PORTA_PIN2          2U              /**< Port A - Pin 2 index. */
#define PORTA_PIN3          3U              /**< Port A - Pin 3 index. */
#define PORTA_PIN6          6U              /**< Port A - Pin 6 index. */
#define PORTA_PIN7          7U              /**< Port A - Pin 7 index. */
#define PORTA_PIN8          8U              /**< Port A - Pin 8 index. */
#define PORTA_PIN9          9U              /**< Port A - Pin 9 index. */
#define PORTA_PIN10         10U             /**< Port A - Pin 10 index. */
#define PORTA_PIN11         11U             /**< Port A - Pin 11 index. */
#define PORTA_PIN12         12U             /**< Port A - Pin 12 index. */

#define PORTB_PIN0          16U             /**< Port B - Pin 0 index (Linear mapping offset 16). */
#define PORTB_PIN6          22U             /**< Port B - Pin 6 index (Linear mapping offset 16). */
#define PORTB_PIN14         30U             /**< Port B - Pin 14 index (Linear mapping offset 16). */
#define PORTB_PIN15         31U             /**< Port B - Pin 15 index (Linear mapping offset 16). */

#define PORTC_PIN13         45U             /**< Port C - Pin 13 index (Linear mapping offset 32). */

/* On-board Basic IO Mappings */
#define LED                 PORTC_PIN13     /**< Target assignment for on-board Status LED (PC13). */
#define BUTTON              PORTA_PIN3      /**< Target assignment for external User Button (PA3). */

/* DC Motor Driver Interface Mappings (H-Bridge Control) */
#define MOTOR_PWM_PIN       PORTA_PIN8      /**< DC Motor speed control via PWM line (TIM1_CH1). */
#define MOTOR_IN1           PORTB_PIN14     /**< H-Bridge direction steering output line 1. */
#define MOTOR_IN2           PORTB_PIN15     /**< H-Bridge direction steering output line 2. */
#define PWM_FREQ            20000U          /**< Ultrasonic PWM carrier frequency set to 20kHz to prevent acoustic hum. */

/* Quadrature Encoder Interface Mappings */
#define MOTOR_ENCODER_A     PORTA_PIN6      /**< Encoder Channel A capture phase input (TIM3_CH1). */
#define MOTOR_ENCODER_B     PORTA_PIN7      /**< Encoder Channel B capture phase input (TIM3_CH2). */

/* RC Servo Motor Interface Mappings */
#define SERVO_PWM_PIN       PORTB_PIN6      /**< Servo position steering control via PWM line (TIM4_CH1). */
#define SERVO_FREQ          50U             /**< Standard RC Servo operating base frequency update clock rate (50Hz). */

/* USART1 Serial Telemetry Interface Mappings */
#define USART1_TX_PIN       PORTA_PIN9      /**< USART1 synchronous data wire transmitter pin source (PA9). */
#define USART1_RX_PIN       PORTA_PIN10     /**< USART1 synchronous data wire receiver pin source (PA10). */

/* Controller Area Network (bxCAN) Interface Mappings */
#define CAN_BAUDRATE_SUP    500000U         /**< Enforced nominal operational speed limit factor set at 500kbps. */
#define CAN_RX_PIN          PORTA_PIN11     /**< bxCAN network transceiver receiver connection pathway pin (PA11). */
#define CAN_TX_PIN          PORTA_PIN12     /**< bxCAN network transceiver transmitter connection pathway pin (PA12). */

/*******************************************************************************
 * API
 ******************************************************************************/

/**
 * @brief  Executes a synchronized master boot sequence initializing all system hardware modules.
 * @note   Internally calls individual peripheral driver setups (RCC, GPIO, Timers, USART, CAN)
 * to bring the board up to full operational ready status.
 * @return None
 */
void BSP_Init(void);

#ifdef __cplusplus
}
#endif

#endif /* BSP_CONFIG_H_ */