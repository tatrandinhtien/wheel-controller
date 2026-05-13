INC += -Ios/FreeRTOS/include
INC += -Ios/FreeRTOS/portable/GCC/ARM_CM3


SRCS +=	os/FreeRTOS/croutine.c					\
		os/FreeRTOS/event_groups.c  			\
		os/FreeRTOS/list.c          			\
		os/FreeRTOS/queue.c         			\
		os/FreeRTOS/stream_buffer.c 			\
		os/FreeRTOS/tasks.c         			\
		os/FreeRTOS/timers.c					\
		os/FreeRTOS/portable/MemMang/heap_4.c	\
		os/FreeRTOS/portable/GCC/ARM_CM3/port.c
