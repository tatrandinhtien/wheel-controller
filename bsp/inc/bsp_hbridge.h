/**
 * @file bsp_hbridge.h
 * @brief BSP H-Bridge API (stub)
 */

#ifndef BSP_HBRIDGE_H_
#define BSP_HBRIDGE_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

void BSP_HBridge_Init(void);
void BSP_HBridge_SetSpeed(float percentage);

#ifdef __cplusplus
}
#endif

#endif /* BSP_HBRIDGE_H_ */
