#include "bsp_hbridge.h"

// Minimal H-bridge stub. Replace with hardware-specific implementation.
void BSP_HBridge_Init(void) {
    // TODO: configure direction GPIOs and PWM timer
}

void BSP_HBridge_SetSpeed(float percentage) {
    // percentage: -100.0 .. 100.0
    // TODO: set IN1/IN2 and PWM duty
    (void)percentage;
}
