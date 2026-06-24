/**
 * @file    bsp_encoder.h
 * @author  DinhTien (tien.ta.eswe@gmail.com)
 * @brief   Board Support Package (BSP) for Quadrature Encoder interface.
 * @details Wraps low-level Timer Input Capture configurations into functional APIs 
 * to extract speed feedback (delta ticks) and odometer data (total steps) 
 * for motor velocity and position control loops.
 * @version 1.0
 * @date    2026-06-23
 *
 * @copyright Copyright (c) 2026
 *
 */

#ifndef BSP_ENCODER_H
#define BSP_ENCODER_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

/*******************************************************************************
 * API
 ******************************************************************************/

/**
 * @brief  Initializes the hardware Timer peripheral in Quadrature Encoder mode.
 * @note   Configures target input channels with noise glitch filters and enables 
 * simultaneous edge tracking (X4 decoding mode).
 * @return None
 */
void BSP_Encoder_Init(void);

/**
 * @brief  Retrieves the fractional change in encoder pulses (ticks) since the last call.
 * @note   This delta calculation automatically handles 16-bit timer counter overflows
 * and underflows, returning signed velocity directional steps.
 * @return int16_t: Signed pulse difference (positive for forward, negative for reverse).
 */
int16_t BSP_Encoder_GetDelta(void);

/**
 * @brief  Computes the absolute accumulated position counts from system boot baseline.
 * @details Integrates historical delta updates into a robust 32-bit tracking window 
 * acting as the odometer for physical wheel distance calculations.
 * @return int32_t: Total absolute accumulated encoder ticks.
 */
int32_t BSP_Encoder_GetTotalPosition(void);

#ifdef __cplusplus
}
#endif

#endif /* BSP_ENCODER_H */