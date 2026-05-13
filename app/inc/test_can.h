/**
 * @file test_can.h
 * @brief Application application to test CAN driver and BSP
 */

#ifndef TEST_CAN_H_
#define TEST_CAN_H_

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Run a simple loopback test for the CAN Bus
 */
void test_can_run(void);

#ifdef __cplusplus
}
#endif

#endif /* TEST_CAN_H_ */