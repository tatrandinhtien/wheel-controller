#include "bsp_encoder.h"

// Minimal encoder stub. Replace with hardware-specific implementation.
static volatile int32_t last_count = 0;

void BSP_Encoder_Init(void) {
    // TODO: Call driver to configure timer in encoder interface mode
}

float BSP_Encoder_GetSpeedRPM(void) {
    // TODO: Read TIMx->CNT and compute RPM using configured timer prescaler
    (void)last_count;
    return 0.0f;
}
