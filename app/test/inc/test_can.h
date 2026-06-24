/**
 * @file    test_can.h
 * @author  DinhTien (tien.ta.eswe@gmail.com)
 * @brief   Testing framework interface for Master/Slave CAN bus communication.
 * @version 1.0
 * @date    2026-06-24
 *
 * @copyright Copyright (c) 2026
 *
 */

#ifndef TEST_CAN_H_
#define TEST_CAN_H_

#ifdef __cplusplus
extern "C" {
#endif

/*******************************************************************************
 * API
 ******************************************************************************/

/**
 * @brief  Initializes system clocks, boots the bxCAN driver layer, and launches the active role task.
 * @note   Depending on CURRENT_NODE_ROLE macro evaluation, this routing spins up either 
 * a Master Command Generator or a Slave Responder thread under FreeRTOS.
 * @return None
 */
void test_can_run(void);

#ifdef __cplusplus
}
#endif

#endif /* TEST_CAN_H_ */