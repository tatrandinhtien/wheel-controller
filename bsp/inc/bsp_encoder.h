/**
 * @file bsp_encoder.h
 * @brief BSP Encoder API (stub)
 */

#ifndef BSP_ENCODER_H_
#define BSP_ENCODER_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

void BSP_Encoder_Init(void);
float BSP_Encoder_GetSpeedRPM(void);

#ifdef __cplusplus
}
#endif

#endif /* BSP_ENCODER_H_ */
