#include "bsp_servo.h"

// Minimal servo stub. Replace with hardware-specific implementation.
void BSP_Servo_Init(void) {
    // TODO: configure timer for 50Hz PWM and set default position
}

void BSP_Servo_SetAngle(uint8_t angle) {
    // angle: 0..180 -> map to 0.5ms..2.5ms
    (void)angle;
}
