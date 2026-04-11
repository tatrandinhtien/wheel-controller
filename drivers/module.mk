INCLUDE	+= -Idrivers/inc
INCLUDE += -Idrivers/core/inc


SRCS += drivers/src/Driver_GPIO.c  	\
		drivers/src/Driver_USART.c 	\
		drivers/src/Driver_RCC.c   	\
		drivers/src/Driver_Timer.c	\
		drivers/core/src/system_stm32f1xx.c