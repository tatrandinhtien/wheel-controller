# STM32F103B Motor Control Project - Context Document

**Author:** Tiến Tạ  
**Email:** tien.ta.eswe@gmail.com  
**Microcontroller:** STM32F103xB (Cortex-M3, 72 MHz)  
**Build System:** Make-based embedded C project  
**OS:** FreeRTOS v10.4+  

---

## 1. Project Overview

This is an **embedded motor control system** designed for a wheeled autonomous vehicle/robot on the STM32F103 microcontroller. The project implements:

- **DC Motor Control** with PWM speed regulation
- **Quadrature Encoder Feedback** for position/speed measurement
- **H-Bridge Motor Driver** (e.g., ddri0042) control
- **Servo Control** for steering/manipulation
- **RS485 Communication** (modular slave architecture)
- **Real-time Task Management** using FreeRTOS preemptive scheduler
- **UART Console Interface** for debugging/monitoring

### Target Hardware
- **Microcontroller:** STM32F103xB (144-pin LQFP)
- **Crystal:** 8 MHz (PLL → 72 MHz system clock)
- **Peripherals:** Timer (PWM + Quadrature), USART, GPIO, RCC
- **Programmers:** ST-Link v2 (via OpenOCD)

---

## 2. Architecture & Layering

The project follows a **3-tier layered architecture** for clean separation of concerns:

```
┌─────────────────────────────────────┐
│   Application Layer (app/)          │  - Test modules, main logic
│   - main.c, test_*.c                │  - Feature validation
│   - Conditional compilation tests   │
└────────────┬────────────────────────┘
             │
┌────────────▼────────────────────────┐
│   Board Support Package (bsp/)      │  - Hardware abstraction
│   - Motor control (H-Bridge)        │  - Encoder readout
│   - Servo control                   │  - Console UART
│   - Encoder quadrature decoding     │  - Reusable device drivers
│   - Abstracts Driver layer          │
└────────────┬────────────────────────┘
             │
┌────────────▼────────────────────────┐
│   Hardware Driver Layer (drivers/)  │  - Peripheral registers
│   - Driver_GPIO.c/.h                │  - Low-level HAL
│   - Driver_Timer.c/.h               │  - Direct HW control
│   - Driver_USART.c/.h               │  - Register configuration
│   - Driver_RCC.c/.h                 │
│   - STM32F1xx CMSIS headers         │
└────────────┬────────────────────────┘
             │
┌────────────▼────────────────────────┐
│   Real-time OS (os/FreeRTOS/)       │  - Task scheduler
│   - Task management                 │  - Synchronization
│   - Queue/Semaphore support         │  - Timer ticks (1 kHz)
│   - Preemptive multitasking         │  - Hardware port (ARM CM3)
└─────────────────────────────────────┘
```

### Design Principles
- **Hardware Independence:** Application code doesn't directly manipulate registers
- **Reusability:** BSP components can be reused across projects
- **Testability:** Each layer has test modules in app/src/test_*.c
- **Configuration:** Compile-time feature selection via `test_config.h`

---

## 3. Project Structure & Responsibilities

### 📂 **app/** - Application & Test Layer
**Purpose:** High-level application logic and module testing

```
app/
├── src/
│   ├── main.c                 # Main entry point, test dispatcher
│   ├── ring_buffer.c          # Circular buffer for UART data
│   ├── test_*.c               # Feature validation modules
│   │   ├── test_gpio.c        # GPIO toggle test
│   │   ├── test_timer.c       # PWM & timer test
│   │   ├── test_console.c     # UART communication test
│   │   └── test_gpio.c        # (repeated reference)
│   
└── inc/
    ├── main.h
    ├── test_config.h          # Conditional compilation flags:
    │                           # GPIO_TEST, TIMER_TEST, CONSOLE_TEST
    ├── FreeRTOSConfig.h       # OS config (72 MHz, 1 kHz tick, 10 KB heap)
    ├── ring_buffer.h
    └── test_*.h               # Test module headers
```

**Key Config Constants:**
- `LED = 45` → PC13 (onboard LED)
- `BUTTON = 10` → PA10 (input button)
- `MOTOR_PWM_PIN = 8` → PA8 (motor speed control)
- `MOTOR_ENCODER_A = 0`, `MOTOR_ENCODER_B = 1` → PA0, PA1 (feedback)
- `PORTB_PIN10 = 26`, `PORTB_PIN11 = 27` → PB10, PB11 (servo/other peripherals)

---

### 📂 **bsp/** - Board Support Package (Hardware Abstraction)
**Purpose:** Reusable device drivers hiding Driver layer complexity

```
bsp/
├── src/
│   ├── console.c              # UART serial communication
│   ├── h_bridge.c             # DC motor H-Bridge control (ddri0042)
│   │                           # - Handles PWM + direction logic
│   ├── encoder.c              # Quadrature decoder (STM32 HW)
│   │                           # - Overflow/underflow handling
│   ├── servo.c                # Servo PWM control (0-180°)
│   
└── inc/
    ├── console.h              # void console_init()
    ├── h_bridge.h             # void BSP_HBridge_Init()
    │                           # void BSP_HBridge_SetSpeed(int16_t %)
    │                           # void BSP_HBridge_Brake()
    ├── encoder.h              # void BSP_Encoder_Init()
    │                           # int16_t BSP_Encoder_GetDelta()
    │                           # void BSP_Encoder_Reset()
    └── servo.h                # Servo control APIs
```

**Key Abstractions:**
- `BSP_HBridge_SetSpeed(speed_percent)` → Auto-handles ±100% → PWM + direction pins
- `BSP_Encoder_GetDelta()` → Returns delta pulses since last call (signed)
- `BSP_Encoder_Reset()` → Clear counter for new motion cycle

---

### 📂 **drivers/** - Low-level Hardware Drivers
**Purpose:** Direct microcontroller peripheral access

```
drivers/
├── core/                      # CMSIS & vendor headers
│   ├── inc/
│   │   ├── stm32f103xb.h     # Register definitions
│   │   ├── core_cm3.h         # ARM Cortex-M3 internals
│   │   ├── cmsis_*.h          # Compiler abstractions
│   │   └── system_stm32f1xx.h # SystemClock setup
│   └── src/
│       └── system_stm32f1xx.c # Clock initialization
│
└── inc/ + src/                # Application-level drivers
    ├── Driver_RCC.h/.c        # Reset & Clock Control (clock setup)
    ├── Driver_GPIO.h/.c       # GPIO pin configuration
    ├── Driver_Timer.h/.c      # PWM, capture, quadrature modes
    ├── Driver_USART.h/.c      # Serial communication
    ├── Driver_CAN.h/.c        # CAN (if applicable)
    └── Driver_Common.h        # Shared enums/macros
```

---

### 📂 **os/FreeRTOS/** - Real-time Operating System
**Purpose:** Multi-tasking kernel and synchronization primitives

```
os/FreeRTOS/
├── include/
│   ├── FreeRTOS.h            # Main kernel header
│   ├── task.h                # Task creation/management
│   ├── queue.h               # Inter-task queues
│   ├── semphr.h              # Semaphores/mutexes
│   ├── timers.h              # Software timers
│   └── ...other modules
│
├── src/
│   ├── tasks.c               # Task scheduler implementation
│   ├── queue.c, list.c       # Core data structures
│   ├── portable/             # ARM Cortex-M3 port
│   └── MemMang/              # Memory allocators
│
└── module.mk                 # FreeRTOS build config
```

**Configuration (FreeRTOSConfig.h):**
- **Clock:** 72 MHz
- **Tick Rate:** 1 kHz (1 ms resolution)
- **Max Priorities:** 5 levels
- **Heap Size:** 10 KB
- **Preemption:** Enabled

---

### 📂 **Example/** - Legacy Arduino Code Reference
**Purpose:** Contains old implementation on Arduino Micro Pro for reference/migration

```
Example/
├── Slave_test_Micro_10.4.ino  # Main Arduino sketch (OLD)
│                               # - RS485 slave communication
│                               # - Motor control (PID)
│                               # - Encoder handling
│                               # - Servo control
│
├── Send_Recieve_data.h        # RS485 protocol definitions
└── timer.h                    # Arduino timer setup
```

**Important Note:** This folder is **legacy code** from a previous Arduino Micro Pro implementation. Use it as a **reference for logic/algorithm** but do NOT directly port it—the STM32 version uses different timers, ISRs, and peripherals.

**Key Logic to Port (if needed):**
- **PID Control:** Proportional (Kp=4), Integral (Ki=3), Derivative (Kd=0.001), ΔT=0.02s
- **Motor Speed Calculation:** RPM from encoder pulses with scaling factor K=255863.5394
- **Encoder Modes:** Channel A/B quadrature with overflow handling
- **RS485 Slave Protocol:** Master sends commands, slave responds with status

---

### 📂 **Root Files**
- **Makefile** → Build system (arm-none-eabi-gcc, modular includes)
- **my_linker_script.ld** → Memory layout for STM32F103xB
- **my_startup_code.s** → Reset handler and vector table
- **STM32F103.svd** → Peripheral register description (IDE reference)
- **.clang-format** → Code style guidelines
- **.git/.gitignore** → Version control

---

## 4. Current Development Status

### ✅ Completed Modules
- [x] **CMSIS Core** - ARM Cortex-M3 headers
- [x] **System Clock** - 72 MHz PLL initialization
- [x] **GPIO Driver** - Pin configuration
- [x] **Timer Driver** - PWM and quadrature modes
- [x] **USART Driver** - Serial communication
- [x] **FreeRTOS** - Task scheduler + preemption
- [x] **Ring Buffer** - UART data buffering
- [x] **H-Bridge Abstraction** - Motor direction + PWM

### 🔄 In Progress / To Do
- [ ] **Complete Encoder BSP** - Finalize overflow handling algorithm
- [ ] **Motor PID Control** - Implement feedback loop (reference: Example/)
- [ ] **Servo Control BSP** - PWM angle mapping (0-180°)
- [ ] **RS485 Slave Protocol** - Multi-device communication (from Example/)
- [ ] **Advanced Testing** - Integration tests on actual hardware
- [ ] **Documentation** - Doxygen comments for all BSP APIs
- [ ] **Optimization** - Performance profiling, interrupt latency

---

## 5. Build & Flash Instructions

```bash
# Clean all build artifacts
make clean

# Build the project
make all
# Generates: build/my_project.elf, build/my_project.bin, build/my_project.map

# Flash to STM32F103xB via ST-Link
make flash
# Uses OpenOCD with ST-Link v2 interface

# Show binary size
make print_size
```

**Build Output:**
- **ELF:** `build/my_project.elf` (for debugging)
- **BIN:** `build/my_project.bin` (for flashing)
- **MAP:** `build/my_project.map` (for symbol analysis)

---

## 6. Compilation Features (test_config.h)

Enable/disable tests at compile time:

```c
#define GPIO_TEST    ENABLE   // Test LED toggle on PC13
#define TIMER_TEST   ENABLE   // Test PWM on PA8
#define CONSOLE_TEST ENABLE   // Test USART communication
```

When enabled, tests run in `main()` before entering any infinite loop or OS scheduler.

---

## 7. Hardware Pin Mapping

| Function | STM32 Pin | Arduino Label (ref) | Mode | Driver |
|----------|-----------|-------------------|------|--------|
| LED | PC13 | - | GPIO Output | Driver_GPIO |
| Button | PA10 | - | GPIO Input (Pull-up) | Driver_GPIO |
| Motor PWM | PA8 | 9 | Timer PWM | Driver_Timer |
| Encoder A | PA0 | 2 | Timer Quadrature | Driver_Timer |
| Encoder B | PA1 | 3 | Timer Quadrature | Driver_Timer |
| Servo PWM | PA9 | 5 | Timer PWM | Driver_Timer |
| H-Bridge IN1 | PB6 | 6 | GPIO Output | Driver_GPIO |
| H-Bridge IN2 | PB7 | 7 | GPIO Output | Driver_GPIO |
| RS485 TX | PA9 | TX1 | USART TX | Driver_USART |
| RS485 RX | PA10 | RX1 | USART RX | Driver_USART |
| RS485 EN | PA4 | 4 | GPIO Output | Driver_GPIO |

---

## 8. Key Data Structures & Macros

From **test_config.h:**
```c
#define ENABLE     1
#define DISABLE    0
#define ON         0        // Active low for LED
#define OFF        1
#define HIGH       1
#define LOW        0

// Pin aliases (global references, NOT pin numbers)
#define LED               45  // PC13
#define BUTTON            10  // PA10
#define MOTOR_PWM_PIN     8   // PA8
#define MOTOR_ENCODER_A   0   // PA0
#define MOTOR_ENCODER_B   1   // PA1
```

---

## 9. Debugging & Tools

- **VS Code** - IDE with C/C++ extension
- **OpenOCD** - Debugger/flasher (ST-Link v2)
- **arm-none-eabi-gcc** - Cross-compiler toolchain
- **CMSIS-DAP / ST-Link Probe** - Hardware programmer
- **STM32CubeMX** (optional) - Peripheral configuration tool

---

## 10. Design Notes for Future Contributors

### Best Practices
1. **Never skip the BSP layer** - Always go through bsp/* for hardware tasks
2. **Use FreeRTOS primitives** - Queues, semaphores for task sync, not busy-wait
3. **Keep interrupt handlers short** - Defer heavy processing to tasks
4. **Test incrementally** - Use the test_*.c modules for validation
5. **Document hardware requirements** - Pin usage, clock sources, interrupt priorities

### Known Limitations
- **Heap Size:** 10 KB - careful with large data structures
- **Max Priorities:** 5 - prioritize PID control > encoder > servo > communication
- **32-bit Timers:** Watch for overflow at high frequencies
- **UART Speed:** RS485 @ 500 kbps may need optimization under heavy FreeRTOS load

### Migration Notes (Arduino → STM32)
The `Example/` folder contains Arduino Micro Pro code. Key differences:
- Arduino's `attachInterrupt()` → STM32 NVIC setup in Driver_Timer
- Arduino PWM via `analogWrite()` → STM32 TIMx registers via Driver_Timer
- Arduino serial `Serial.print()` → STM32 USART via Driver_USART + Ring Buffer
- Arduino millis() → FreeRTOS xTaskGetTickCount() (1 ms resolution)

---

## Quick Links to Key Files

- [Main Application](app/src/main.c)
- [Test Configuration](app/inc/test_config.h)
- [Build System](Makefile)
- [FreeRTOS Config](app/inc/FreeRTOSConfig.h)
- [H-Bridge API](bsp/inc/h_bridge.h)
- [Encoder API](bsp/inc/encoder.h)
- [Legacy Arduino Reference](Example/Slave_test_Micro_10.4.ino)

---

**Last Updated:** May 13, 2026
