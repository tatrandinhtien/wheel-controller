/**
 * @file    app_wheel_controller.c
 * @author  DinhTien (tien.ta.eswe@gmail.com)
 * @brief   Distributed smart wheel node controller implementation under FreeRTOS.
 * @version 1.0
 * @date    2026-06-23
 *
 * @copyright Copyright (c) 2026
 *
 */

#include <stdio.h>
#include "cmsis_os2.h"
#include "bsp_encoder.h"
#include "bsp_motor.h"
#include "bsp_servo.h"
#include "bsp_can.h"
#include "bsp_log.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/

#define NODE_RL_WHEEL     

#if defined(NODE_FL_WHEEL)
    #define RX_OFFSET       0       
    #define TX_FEEDBACK_ID  0x200   
    #define SERVO_TUNE      0       
#elif defined(NODE_FR_WHEEL)
    #define RX_OFFSET       2       
    #define TX_FEEDBACK_ID  0x210   
    #define SERVO_TUNE      -250    
#elif defined(NODE_RL_WHEEL)
    #define RX_OFFSET       4       
    #define TX_FEEDBACK_ID  0x220   
    #define SERVO_TUNE      -500    
#elif defined(NODE_RR_WHEEL)
    #define RX_OFFSET       6       
    #define TX_FEEDBACK_ID  0x230   
    #define SERVO_TUNE      -250    
#else
    #error "YOU MUST SELECT A WHEEL NODE IDENTITY!"
#endif

#define TOTAL_PPR           (11.0f * 4.0f * 45.0f) 
#define TS_DRIVE_MS         10U     
#define TS_DRIVE_SEC        0.01f   
#define TS_CAN_TX_MS        100U    
#define CAN_FAILSAFE_MS     500U    

typedef struct {
    int16_t raw_rx_speed;
    int16_t raw_rx_angle;
    int16_t actual_rpm_x10;
    uint32_t last_rx_ticks;
} VehicleState_t;

/*******************************************************************************
 * Variables
 ******************************************************************************/

volatile VehicleState_t node = {0, 0, 0, 0};

/*******************************************************************************
 * Prototypes
 ******************************************************************************/

static void vCAN_RxTask(void *argument);
static void vDriveCtrlTask(void *argument);
static void vCAN_TxTask(void *argument);

/*******************************************************************************
 * Code
 ******************************************************************************/

/**
 * @brief  High-priority asynchronous thread capturing broadcast master frame structures.
 * @param  argument: Unused protocol pointer reference matching CMSIS prototypes.
 * @return None
 */
static void vCAN_RxTask(void *argument) {
    (void)argument;
    CAN_Message_t rx_msg;
    
    while(1) {
        if (BSP_CAN_Read(&rx_msg, osWaitForever)) {
            /* Stamp time log to keep the hardware watchdog/failsafe pipeline alive */
            node.last_rx_ticks = osKernelGetTickCount();
            
            /* Deserialize 16-bit parameters from designated array indices */
            if (rx_msg.id == 0x100) {
                node.raw_rx_speed = (int16_t)(rx_msg.data[RX_OFFSET] | (rx_msg.data[RX_OFFSET + 1] << 8));
            }
            else if (rx_msg.id == 0x101) {
                node.raw_rx_angle = (int16_t)(rx_msg.data[RX_OFFSET] | (rx_msg.data[RX_OFFSET + 1] << 8));
            }
        }
    }
}

/**
 * @brief  Deterministic 100Hz control loop coordinating sensor feedback and actuator tracking.
 * @param  argument: Unused protocol pointer reference matching CMSIS prototypes.
 * @return None
 */
static void vDriveCtrlTask(void *argument) {
    (void)argument;
    uint32_t tick_start = osKernelGetTickCount();
    const uint32_t tick_freq = (TS_DRIVE_MS * osKernelGetTickFreq()) / 1000U;
    const uint32_t failsafe_ticks = (CAN_FAILSAFE_MS * osKernelGetTickFreq()) / 1000U;

    while(1) {
        int16_t speed_cmd = node.raw_rx_speed;
        int16_t angle_cmd = node.raw_rx_angle;

        /* Network Failsafe Condition: Kill motor effort if Master signal drops offline */
        if ((osKernelGetTickCount() - node.last_rx_ticks) > failsafe_ticks) {
            speed_cmd = 0;
        }

        /* Forward scaled efforts straight down onto the H-Bridge hardware registers */
        int16_t motor_out = speed_cmd / 10;
        BSP_Motor_SetSpeed(motor_out); 

        /* Enforce physical structural limits onto angular directives */
        int16_t servo_base = angle_cmd / 10;
        if (servo_base > 900)  servo_base = 900;
        if (servo_base < -900) servo_base = -900;
        
        /* Apply physical steering offset tuning metrics */
        int16_t final_angle = servo_base + SERVO_TUNE;
        BSP_Servo_SetAngle(final_angle);

        /* Capture localized encoder delta ticks and scale into high-resolution RPM */
        int16_t delta = BSP_Encoder_GetDelta(); 
        float actual_rpm = ((float)delta * 60.0f) / (TOTAL_PPR * TS_DRIVE_SEC);
        node.actual_rpm_x10 = (int16_t)(actual_rpm * 10.0f);

        tick_start += tick_freq;
        osDelayUntil(tick_start);
    }
}

/**
 * @brief  Periodic 10Hz telemetry thread serialization local states back to Master nodes.
 * @param  argument: Unused protocol pointer reference matching CMSIS prototypes.
 * @return None
 */
static void vCAN_TxTask(void *argument) {
    (void)argument;
    uint32_t tick_start = osKernelGetTickCount();
    const uint32_t tick_freq = (TS_CAN_TX_MS * osKernelGetTickFreq()) / 1000U;
    
    CAN_Message_t tx_msg;
    tx_msg.id = TX_FEEDBACK_ID;
    tx_msg.dlc = 8;
    tx_msg.isExt = 0; 
    tx_msg.isRTR = 0; 
    for(int i = 0; i < 8; i++) tx_msg.data[i] = 0;

    while(1) {
        /* Serialize internal variables sequentially into single byte arrays */
        tx_msg.data[0] = node.raw_rx_speed & 0xFF;
        tx_msg.data[1] = (node.raw_rx_speed >> 8) & 0xFF;
        tx_msg.data[2] = node.raw_rx_angle & 0xFF;
        tx_msg.data[3] = (node.raw_rx_angle >> 8) & 0xFF;
        tx_msg.data[4] = node.actual_rpm_x10 & 0xFF;
        tx_msg.data[5] = (node.actual_rpm_x10 >> 8) & 0xFF;

        /* Issue safe non-blocking write commands */
        BSP_CAN_Write(&tx_msg, 100);
        
        tick_start += tick_freq;
        osDelayUntil(tick_start);
    }
}

void app_wheel_controller(void) {
    /* Initialize baseline board support peripherals */
    BSP_Encoder_Init();
    BSP_Motor_Init();
    BSP_Servo_Init();
    BSP_CAN_Init();
    bsp_log_init();

    setvbuf(stdout, NULL, _IONBF, 0);
    osKernelInitialize();

    /* Establish concurrent multi-task execution pipelines */
    const osThreadAttr_t rx_attr = { .name = "CAN_Rx", .priority = osPriorityHigh, .stack_size = 1024 };
    osThreadNew(vCAN_RxTask, NULL, &rx_attr);

    const osThreadAttr_t drv_attr = { .name = "Drive", .priority = osPriorityAboveNormal, .stack_size = 2048 };
    osThreadNew(vDriveCtrlTask, NULL, &drv_attr);

    const osThreadAttr_t tx_attr = { .name = "CAN_Tx", .priority = osPriorityNormal, .stack_size = 1024 };
    osThreadNew(vCAN_TxTask, NULL, &tx_attr);

    osKernelStart();
    while (1) {}
}