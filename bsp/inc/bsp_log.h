/**
 * @file    bsp_log.h
 * @author  DinhTien (tien.ta.eswe@gmail.com)
 * @brief   Board Support Package (BSP) for System Console and Logging.
 * @details Provides abstraction layer APIs for system debugging and telemetry 
 * output via USART. Implements a thread-safe formatted print utility 
 * supporting asynchronous queues for real-time status reporting.
 * @version 1.0
 * @date    2026-06-23
 *
 * @copyright Copyright (c) 2026
 *
 */

#ifndef BSP_LOG_H_
#define BSP_LOG_H_

#ifdef __cplusplus
extern "C" {
#endif

/*******************************************************************************
 * API
 ******************************************************************************/

/**
 * @brief  Initializes the telemetry console peripheral, queues, semaphores, and worker task.
 * @note   Configures the underlying USART1 peripheral with standard framing 
 * (8 data bits, no parity, 1 stop bit, 115200 bps) and triggers OS primitives.
 * @return None
 */
void bsp_log_init(void);

/**
 * @brief  Prints a formatted string asynchronously to the system serial console.
 * @details Formats string input using vsnprintf and pushes it into an internal 
 * RTOS message queue. 
 * @note   Non-blocking design. Maximum length per log message is capped at 64 characters 
 * (including null terminator). Longer strings will be safely truncated. If the queue is 
 * full, the message is dropped immediately to maintain real-time determinism.
 * @param  format: Pointer to a null-terminated string specifying the data format layout.
 * @param  ...: Variadic arguments matching the format specifiers.
 * @return None
 */
void bsp_log_printf(const char *format, ...);

#ifdef __cplusplus
}
#endif

#endif /* BSP_LOG_H_ */