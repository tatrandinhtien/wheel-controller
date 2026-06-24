/**
 * @file    ring_buffer.h
 * @author  DinhTien (tien.ta.eswe@gmail.com)
 * @brief   Board Support Package (BSP) for Power-of-2 Ring Buffer.
 * @details Provides a lock-free, single-producer single-consumer (SPSC) ring buffer
 * implementation. Utilizes bitwise mask wrapping mechanics instead of expensive 
 * modulo arithmetic operations to safely push and pop single-byte sequences.
 * @version 1.0
 * @date    2026-06-23
 *
 * @copyright Copyright (c) 2026
 *
 */

#ifndef RING_BUFFER_H_
#define RING_BUFFER_H_

#ifdef __cplusplus
extern "C"
{
#endif

#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>

/*******************************************************************************
 * Definitions
 ******************************************************************************/

/**
 * @brief Ring buffer tracking structure container instance.
 */
typedef struct {
    uint8_t *pdata;             /**< Pointer mapping onto the physical underlying memory storage array. */
    volatile uint16_t head;     /**< Write index pointer tracking incoming data entry boundary slots. */
    volatile uint16_t tail;     /**< Read index pointer tracking outgoing data extraction boundary slots. */
    uint16_t capacity;          /**< Maximum storage slots window length (Enforced strictly to Power-of-2 sizes). */
} ring_buffer_t;

/*******************************************************************************
 * API
 ******************************************************************************/

/**
 * @brief  Initializes the ring buffer tracking parameters and links raw storage buffers.
 * @note   The buffer capacity parameter length MUST be a strict power-of-2 number configuration 
 * (e.g., 16, 32, 64, 128, 256) to satisfy optimal bitwise alignment masks.
 * @param  rb: Destination reference pointer block to the ring buffer handle structure.
 * @param  data: Target storage array block to be managed as a FIFO pool.
 * @param  len: Total storage length size allocation.
 * @retval true: Initialization success.
 * @retval false: Invalid pointer parameters or length is not a valid Power-of-2.
 */
bool rb_init(ring_buffer_t *rb, uint8_t *data, uint16_t len);

/**
 * @brief  Evaluates whether the ring buffer storage has dropped down to empty bounds.
 * @param  rb: Reference pointer to the targeted active ring buffer structure.
 * @retval true: Buffer is empty (head matches tail).
 * @retval false: Buffer holds unread valid telemetry byte arrays.
 */
bool rb_is_empty(ring_buffer_t *rb);

/**
 * @brief  Evaluates whether the ring buffer storage window is saturated full.
 * @note   To distinguish between complete fullness and complete emptiness flags cleanly, 
 * this design utilizes a single blank guard slot strategy, capping capacity at (len - 1).
 * @param  rb: Reference pointer to the targeted active ring buffer structure.
 * @retval true: Buffer is completely full.
 * @retval false: Free available storage slots remain open for write access hooks.
 */
bool rb_is_full(ring_buffer_t *rb);

/**
 * @brief  Extracts a single data byte sequence out from the circular tracking queue.
 * @param  rb: Reference pointer to the targeted active ring buffer structure.
 * @param  data: Output destination memory container pointer block to drop extracted values into.
 * @retval true: Extraction successful.
 * @retval false: Operation failed because the circular loop buffer context is empty.
 */
bool rb_get(ring_buffer_t *rb, uint8_t *data);

/**
 * @brief  Appends a single data byte sequence straight into the circular tracking queue.
 * @param  rb: Reference pointer to the targeted active ring buffer structure.
 * @param  data: Target input source data value byte to pass into the ring ledger.
 * @retval true: Byte enqueued successfully.
 * @retval false: Operation failed because the circular loop buffer context is full.
 */
bool rb_put(ring_buffer_t *rb, uint8_t data);

#ifdef __cplusplus
}
#endif

#endif /* RING_BUFFER_H_ */