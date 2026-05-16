/**
 * @file bsp_can.h
 * @brief Simple BSP layer for CAN bus
 */

#ifndef BSP_CAN_H_
#define BSP_CAN_H_

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Initialize CAN bus
 * 
 * @param bitrate   Desired bitrate (e.g., 500000)
 */
void BSP_CAN_Init(uint32_t bitrate);

/**
 * @brief Send a CAN message (Standard ID)
 * 
 * @param id        Standard ID
 * @param data      Pointer to data payloadCanSend
 * @param dlc       Data length code (0-8)
 * @return int32_t  Number of bytes sent, or error code (< 0)
 */
int32_t BSP_CAN_Send(uint32_t id, const uint8_t *data, uint8_t dlc);

/**
 * @brief Receive a CAN message from FIFO0
 * 
 * @param id        Output CAN identifier
 * @param data      Buffer to store payload
 * @param size      Maximum buffer size
 * @return int32_t  Number of bytes received, 0 if no message, or error code (< 0)
 */
int32_t BSP_CAN_Receive(uint32_t *id, uint8_t *data, uint8_t size);

#ifdef __cplusplus
}
#endif

#endif /* BSP_CAN_H_ */