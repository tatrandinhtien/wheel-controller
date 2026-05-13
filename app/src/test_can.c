#include "test_can.h"
#include "can_bus.h"
#include "Driver_CAN.h"
#include "console.h"
#include "test_config.h"
#include "Driver_GPIO.h"
#include "Driver_RCC.h"

#include <stdio.h>

#define NODE_ROLE_MASTER 1
#define NODE_ROLE_SLAVE  2

#define CURRENT_NODE_ROLE NODE_ROLE_MASTER

extern ARM_DRIVER_CAN Driver_CAN0;
extern ARM_DRIVER_GPIO Driver_GPIO0;
extern ARM_DRIVER_RCC Driver_RCC0;

#define MASTER_TX_ID 0x111
#define SLAVE_TX_ID  0x222

static void delay(uint32_t time) {
    while (time--) {
        __asm("nop");
    }
}

void test_can_run(void) {
    // Chỉ Initialize Console cho MASTER (vì bạn chỉ có 1 mạch UART to TTL)
#if CURRENT_NODE_ROLE == NODE_ROLE_MASTER
    console_init();
    setvbuf(stdout, NULL, _IONBF, 0);
#endif

    // GPIO Setup for LED on PC13
    RCC_GPIOC_CLK_EN();
    Driver_GPIO0.Setup(LED, NULL);
    Driver_GPIO0.SetDirection(LED, ARM_GPIO_OUTPUT);
    Driver_GPIO0.SetOutputMode(LED, ARM_GPIO_PUSH_PULL);
    Driver_GPIO0.SetOutput(LED, OFF); // Tắt LED (PC13 active low)

    // 2. Initialize CAN at 500kbps
    CAN_Bus_Init(500000);
    
    // Switch to Normal Mode for actual physical bus communication
    Driver_CAN0.SetMode(ARM_CAN_MODE_NORMAL);

#if CURRENT_NODE_ROLE == NODE_ROLE_MASTER

    printf("[INFO] Mode: NORMAL. TX_ID: 0x%X\r\n", MASTER_TX_ID);

    uint8_t tx_data[8] = {1, 0, 0, 0, 0, 0, 0, 0};
    
    while (1) {
        printf("[MASTER] Sending Command: %d ... ", tx_data[0]);
        int32_t sent = CAN_Bus_Send(MASTER_TX_ID, tx_data, 1);
        if (sent > 0) {
            printf("OK!\r\n");
        } else {
            printf("FAILED (Error: %ld)\r\n", sent);
        }

        delay(1000000); // Give slave a moment to reply

        // Receive response from Slave
        ARM_CAN_MSG_INFO rx_info = {0};
        uint8_t rx_data[8] = {0};
        int32_t read_bytes = Driver_CAN0.MessageRead(1, &rx_info, rx_data, 8);
        
        if (read_bytes > 0) {
            printf("[MASTER] Received from 0x%03lX: ", rx_info.id);
            for (int i = 0; i < read_bytes; i++) printf("%02X ", rx_data[i]);
            printf("\r\n");
        }

        // Toggle state for next pulse
        tx_data[0] = (tx_data[0] == 1) ? 0 : 1;

        // Blink master PC13 to indicate it is alive
        static int led_state = ON;
        Driver_GPIO0.SetOutput(LED, led_state);
        led_state = (led_state == ON) ? OFF : ON;

        printf("------------------------------------------\r\n");
        delay(5000000); // interval between packets
    }

#elif CURRENT_NODE_ROLE == NODE_ROLE_SLAVE
    // SLAVE chạy hoàn toàn ẩn (không in Log qua UART)
    
    while (1) {
        ARM_CAN_MSG_INFO rx_info = {0};
        uint8_t rx_data[8] = {0};
        
        int32_t read_bytes = Driver_CAN0.MessageRead(1, &rx_info, rx_data, 8);
        
        if (read_bytes > 0 && rx_info.id == MASTER_TX_ID) {
            uint8_t reply_data[8] = {0xAA, 0xBB, 0x00}; // ACK format

            if (rx_data[0] == 1) {
                Driver_GPIO0.SetOutput(LED, ON);  // Turn ON LED PC13
                reply_data[2] = 1;
            } else {
                Driver_GPIO0.SetOutput(LED, OFF); // Turn OFF LED PC13
                reply_data[2] = 0;
            }

            // Send reply back to Master
            CAN_Bus_Send(SLAVE_TX_ID, reply_data, 3);
        }
    }
#endif
}
