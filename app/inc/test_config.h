#ifndef TEST_CONFIG_H
#define TEST_CONFIG_H

/* PC13 */
#define LED         45

/* PA10 */
#define BUTTON      10

#define ENABLE      1
#define DISABLE     0

#define ON          0
#define OFF         1

#define DELAY_TIME  5000000

/****************************************************************
 * Enable test GPIO function
 ***************************************************************/
#define GPIO_TEST
#define BLINK_LED_TEST
#define BUTTON_INT_TEST

/****************************************************************
 * Enable test Timer function
 ***************************************************************/
#define TIMER_TEST


#endif /* TEST_CONFIG_H */