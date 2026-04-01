INCLUDE	+= -I drivers/inc
INCLUDE += -I drivers/core/inc


SRCS += drivers/src/Driver_CAN.c   \
		drivers/src/Driver_Flash.c \
		drivers/src/Driver_GPIO.c  \
		drivers/src/Driver_USART.c \
		drivers/src/Driver_RCC.c   \
		drivers/core/src/system_stm32f1xx.c