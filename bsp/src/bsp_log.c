/**
 * @file    bsp_log.c
 * @author  DinhTien (tien.ta.eswe@gmail.com)
 * @brief   Board Support Package (BSP) for Logging and Console System.
 * @version 1.0
 * @date    2026-05-30
 *
 * @copyright Copyright (c) 2026
 *
 */

#include "bsp_log.h"
#include "Driver_USART.h"
#include "cmsis_os2.h"

#include <stdio.h>
#include <string.h>
#include <stdarg.h>

/*******************************************************************************
 * Definitions
 ******************************************************************************/

#define QUEUE_SIZE          10
#define LOG_MSG_LEN         64

typedef struct {
    char msg[LOG_MSG_LEN];
} log_packet_t;

/*******************************************************************************
 * Prototypes
 ******************************************************************************/

static void BSP_LogSendDirect(const char *msg);
static void vLoggingTask(void *argument);

/*******************************************************************************
 * Variables
 ******************************************************************************/

static osMessageQueueId_t mid_log_queue;
static osSemaphoreId_t sid_log_sem;
static osThreadId_t tid_log_thread;
static uint8_t bsp_log_ready;

extern ARM_DRIVER_USART Driver_USART1;

/*******************************************************************************
 * Code
 ******************************************************************************/

void USART1_Callback(uint32_t event){
    /* Unblock consumer task once byte sequence shifts completely onto wires */
    if ((event & ARM_USART_EVENT_TX_COMPLETE) && (sid_log_sem != NULL)) {
        osSemaphoreRelease(sid_log_sem);
    }
}

/**
 * @brief  Bypass loop executing raw synchronous blocking write over USART.
 * @param  msg: Null-terminated string source pointing to data.
 * @return None
 */
static void BSP_LogSendDirect(const char *msg) {
    uint32_t len = (uint32_t)strlen(msg);
    uint32_t sent = 0;

    while (sent < len) {
        int32_t res = Driver_USART1.Send(&msg[sent], len - sent);
        if (res > 0) {
            sent += (uint32_t)res;
        }
    }
}

/**
 * @brief  Low-priority RTOS Consumer Task handling asynchronous serial transmission.
 * @param  argument: Unused context payload pointer matching CMSIS prototype.
 * @return None
 */
static void vLoggingTask(void *argument) {
    (void)argument;
    log_packet_t rx;

    while(1) {
        /* Suspend worker thread until a new log packet enters the queue */
        if (osMessageQueueGet(mid_log_queue, &rx, NULL, osWaitForever) == osOK)
        {
            uint32_t msg_len = (uint32_t)strlen(rx.msg);
            if (msg_len == 0) continue;

            /* Fire asynchronous transmission stream */
            if (Driver_USART1.Send(rx.msg, msg_len) == ARM_DRIVER_OK) {
                /* Wait until hardware ISR callback releases the semaphore lock */
                osSemaphoreAcquire(sid_log_sem, osWaitForever);
            }
        }
    }
}

void bsp_log_init(void) {
    /* Allocate queue slots and binary synchronization semaphores */
    mid_log_queue = osMessageQueueNew(QUEUE_SIZE, sizeof(log_packet_t), NULL);
    sid_log_sem = osSemaphoreNew(1, 0, NULL);
    
    const osThreadAttr_t task_attr = {
        .name = "Logging_Task",
        .priority = osPriorityLow,
        .stack_size = 512
    };
    tid_log_thread = osThreadNew(vLoggingTask, NULL, &task_attr);

    /* Verify kernel primitive allocation success */
    if ((mid_log_queue == NULL) || (sid_log_sem == NULL) || (tid_log_thread == NULL)) {
        bsp_log_ready = 0;
        return;
    }

    /* Configure USART1 hardware layer properties: 115200bps, 8N1 */
    if (Driver_USART1.Initialize(USART1_Callback) != ARM_DRIVER_OK) {
        bsp_log_ready = 0;
        return;
    }

    if (Driver_USART1.PowerControl(ARM_POWER_FULL) != ARM_DRIVER_OK) {
        bsp_log_ready = 0;
        return;
    }

    if (Driver_USART1.Control(ARM_USART_MODE_ASYNCHRONOUS | ARM_USART_DATA_BITS_8 | ARM_USART_PARITY_NONE, 115200) != ARM_DRIVER_OK) {
        bsp_log_ready = 0;
        return;
    }

    if (Driver_USART1.Control(ARM_USART_CONTROL_TX, 1) != ARM_DRIVER_OK) {
        bsp_log_ready = 0;
        return;
    }

    bsp_log_ready = 1;
    
    /* Issue immediate boot banner string bypassing the OS queue */
    BSP_LogSendDirect("[bsp_log] ready\r\n");
}

void bsp_log_printf(const char *format, ...) {
    if (!bsp_log_ready || (mid_log_queue == NULL)) {
        return;
    }

    log_packet_t temp;
    va_list args;
    va_start(args, format);
    
    /* Protect memory bounds using vsnprintf to safely truncate long inputs */
    vsnprintf(temp.msg, LOG_MSG_LEN, format, args);
    va_end(args);

    /* Non-blocking push hook: Timeout = 0 forces immediate drop if logging queue floats up */
    osMessageQueuePut(mid_log_queue, &temp, 0, 0); 
}