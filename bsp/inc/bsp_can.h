/**
 * @file    bsp_can.h
 * @author  DinhTien (tien.ta.eswe@gmail.com)
 * @brief   Board Support Package (BSP) for CAN bus communication.
 * @details Wraps the low-level CMSIS CAN driver into simplified APIs 
 * for application layers, providing structured message buffers 
 * with timeout-managed blocking read/write operations.
 * @version 1.0
 * @date    2026-06-23
 *
 * @copyright Copyright (c) 2026
 *
 */

#ifndef BSP_CAN_H
#define BSP_CAN_H

#include <stdint.h>
#include <stdbool.h>

/*******************************************************************************
 * Definitions
 ******************************************************************************/

/**
 * @brief Structure representing a standard or extended CAN frame layout.
 */
typedef struct {
    uint32_t id;            /**< CAN identifier (11-bit standard or 29-bit extended). */
    uint8_t  data[8];       /**< Data payload array (0 to 8 bytes). */
    uint8_t  dlc;           /**< Data Length Code specifying number of valid bytes in payload. */
    uint8_t  isExt;         /**< Extension flag: 1 if Extended ID (29-bit), 0 if Standard ID (11-bit). */
    uint8_t  isRTR;         /**< Remote Transmission Request flag: 1 if RTR frame, 0 if Data frame. */
    uint8_t  reserved;      /**< Reserved byte for structure alignment padding. */
} CAN_Message_t;

/*******************************************************************************
 * API
 ******************************************************************************/

/**
 * @brief  Initializes the CAN hardware instance, OS queues, semaphores, and worker task.
 * @note   Configures bxCAN peripheral at 500kbps, setups Filter Bank 0 onto Object index 3, 
 * and spawns the background Tx task handler thread.
 * @return None
 */
void BSP_CAN_Init(void);

/**
 * @brief  Pushes an application CAN frame onto the asynchronous transmission queue.
 * @param  msg: Reference pointer targeting the source message container to send.
 * @param  timeout_ms: Maximum blocking duration in milliseconds to wait if the queue is full.
 * @retval true: Frame successfully enqueued.
 * @retval false: Failed to enqueue due to timeout limits or missing queue objects.
 */
bool BSP_CAN_Write(CAN_Message_t *msg, uint32_t timeout_ms);

/**
 * @brief  Fetches a parsed received CAN frame out from the local buffer queue.
 * @param  msg: Destination reference pointer block to copy the extracted message data into.
 * @param  timeout_ms: Maximum blocking duration in milliseconds to wait for data arrival.
 * @retval true: Message data retrieved successfully.
 * @retval false: Queue empty or extraction tracking context timed out.
 */
bool BSP_CAN_Read(CAN_Message_t *msg, uint32_t timeout_ms);

#endif /* BSP_CAN_H */