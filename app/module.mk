INC += 	-Iapp/inc \
		-Iapp/test/inc

SRCS += app/src/main.c	\
		app/src/ring_buffer.c	\
		app/src/app_wheel_controller.c \
		app/test/src/test_gpio.c \
		app/test/src/test_log.c \
		app/test/src/test_can.c	\
		app/test/src/test_servo.c \
		app/test/src/test_encoder.c \
		app/test/src/test_motor.c