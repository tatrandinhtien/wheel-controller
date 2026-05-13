/**
 * @file can_bus.h
 * @brief Simple BSP layer for CAN bus
 */

#ifndef CAN_BUS_H_
#define CAN_BUS_H_

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
void CAN_Bus_Init(uint32_t bitrate);

/**
 * @brief Send a CAN message (Standard ID)
 * 
 * @param id        Standard ID
 * @param data      Pointer to data payload
 * @param dlc       Data length code (0-8)
 * @return int32_t  Number of bytes sent, or error code (< 0)
 */
int32_t CAN_Bus_Send(uint32_t id, const uint8_t *data, uint8_t dlc);

#ifdef __cplusplus
}
#endif

#endif /* CAN_BUS_H_ */