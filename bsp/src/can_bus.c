#include "can_bus.h"
#include "Driver_CAN.h"
#include <stddef.h>

extern ARM_DRIVER_CAN Driver_CAN0;

// CAN Event Callback
static void CAN_SignalEvent(uint32_t obj_idx, uint32_t event) {
    // Custom logic to handle event, such as giving a semaphore or queueing RX data
    if (event == ARM_CAN_EVENT_RECEIVE) {
        // Can be handled here
    }
}

void CAN_Bus_Init(uint32_t bitrate) {
    // 1. Initialize CAN Driver
    Driver_CAN0.Initialize(NULL, CAN_SignalEvent);

    // 2. Power on
    Driver_CAN0.PowerControl(ARM_POWER_FULL);

    // 3. Set Bitrate
    Driver_CAN0.SetMode(ARM_CAN_MODE_INITIALIZATION);
    Driver_CAN0.SetBitrate(ARM_CAN_BITRATE_NOMINAL, bitrate, 0);

    // 4. Configure Filter to accept everything on Standard ID
    Driver_CAN0.ObjectSetFilter(0, ARM_CAN_FILTER_ID_MASKABLE_ADD, 0x000, 0x000);

    // 5. Configure RX/TX Objects
    // For STM32F1, Mailbox 0 is often used for TX, FIFO0 for RX
    Driver_CAN0.ObjectConfigure(0, ARM_CAN_OBJ_TX); // Enable TX Interrupt
    Driver_CAN0.ObjectConfigure(1, ARM_CAN_OBJ_RX); // Enable RX Interrupt

    // 6. Enter Normal Mode
    Driver_CAN0.SetMode(ARM_CAN_MODE_NORMAL);
}

int32_t CAN_Bus_Send(uint32_t id, const uint8_t *data, uint8_t dlc) {
    ARM_CAN_MSG_INFO msg_info = {0};
    msg_info.id = id;
    msg_info.rtr = 0;

    return Driver_CAN0.MessageSend(0, &msg_info, data, dlc);
}