#####################################################################
# @author: dt
# @email : tien.ta.eswe@gmail.com
# @date  : 25/03/2026
#####################################################################

TARGET 		= my_project
BUILD_DIR 	= build

PREFIX 		= arm-none-eabi-
CC 			= $(PREFIX)gcc
CXX			= $(PREFIX)g++
AS 			= $(PREFIX)gcc -x assembler-with-cpp
SZ 			= $(PREFIX)size
LD			= $(PREFIX)ld
OBJCOPY 	= $(PREFIX)objcopy
OBJDUMP		= $(PREFIX)objdump

SRCS		=
INC			=
ASM_SRCS	= my_startup_code.s

include app/module.mk
include bsp/module.mk
include drivers/module.mk
include os/FreeRTOS/module.mk

CPU 		= -mthumb -mcpu=cortex-m3
DEFINES 	= -DSTM32F103xB

GENERAL_FLAGS = -O0 -g3 					\
				-Wall 						\
				-fdata-sections 			\
				-ffunction-sections 		\
				-MMD -MP				    \
				--specs=nosys.specs			\
				--specs=nano.specs  		\
				-fsingle-precision-constant

CFLAGS  = $(CPU) $(DEFINES) $(INC) $(GENERAL_FLAGS) -std=c99 -MF"$(@:%.o=%.d)"
CXXFLAGS = $(CPU) $(DEFINES) $(INC) $(GENERAL_FLAGS) -std=c++11 -MF"$(@:%.o=%.d)"

LDFLAGS = $(CPU)									\
		 -Tmy_linker_script.ld 				\
		 -Wl,-Map=$(BUILD_DIR)/$(TARGET).map,--cref	\
		 -Wl,--gc-sections --specs=nano.specs --specs=nosys.specs -u _printf_float

OBJS	 = $(addprefix $(BUILD_DIR)/, $(SRCS:.c=.o))
ASM_OBJS = $(addprefix $(BUILD_DIR)/, $(ASM_SRCS:.s=.o))


all: $(BUILD_DIR)/$(TARGET).elf $(BUILD_DIR)/$(TARGET).bin print_size

$(BUILD_DIR)/$(TARGET).elf: $(OBJS) $(ASM_OBJS)
	@echo "[LINK] $@"
	$(CC) $^ $(LDFLAGS) -o $@

$(BUILD_DIR)/$(TARGET).bin: $(BUILD_DIR)/$(TARGET).elf
	@echo "[BIN] $@"
	$(OBJCOPY) -O binary $< $@

$(BUILD_DIR)/%.o: %.c
	@mkdir -p $(dir $@)
	@echo "[CC] $<"
	$(CC) -c $(CFLAGS) $< -o $@

$(BUILD_DIR)/%.o: %.s
	@mkdir -p $(dir $@)
	@echo "[AS] $<"
	$(AS) -c $(CFLAGS) $< -o $@

print_size: $(BUILD_DIR)/$(TARGET).elf
	@echo ""
	@$(SZ) $<

clean:
	@echo "[CLEAN] Remove directory: $(BUILD_DIR)"
	@rm -rf $(BUILD_DIR)

flash: all
	@echo "[FLASH] Loading code by OpenOCD ..."
	openocd -f interface/stlink.cfg -f target/stm32f1x.cfg -c "program $(BUILD_DIR)/$(TARGET).elf verify reset exit"

-include $(wildcard $(BUILD_DIR)/**/*.d)

.PHONY:	all clean print_size flash
