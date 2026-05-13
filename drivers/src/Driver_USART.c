/**
 */

#include "Driver_USART.h"
#include "Driver_GPIO.h"
#include "Driver_RCC.h"
#include "ring_buffer.h"

#include "stm32f103xb.h"

/********************************************************************
 * Definitions
 ********************************************************************/
#define ARM_USART_DRV_VERSION ARM_DRIVER_VERSION_MAJOR_MINOR(1, 0) /* driver version */
/* Driver Version */
static const ARM_DRIVER_VERSION DriverVersion = {ARM_USART_API_VERSION, ARM_USART_DRV_VERSION};
/* Driver Capabilities */
static const ARM_USART_CAPABILITIES DriverCapabilities = {
    1, /* supports UART (Asynchronous) mode */
    0, /* supports Synchronous Master mode */
    0, /* supports Synchronous Slave mode */
    0, /* supports UART Single-wire mode */
    0, /* supports UART IrDA mode */
    0, /* supports UART Smart Card mode */
    0, /* Smart Card Clock generator available */
    0, /* RTS Flow Control available */
    0, /* CTS Flow Control available */
    0, /* Transmit completed event: \ref ARM_USART_EVENT_TX_COMPLETE */
    0, /* Signal receive character timeout event: \ref ARM_USART_EVENT_RX_TIMEOUT */
    0, /* RTS Line: 0=not available, 1=available */
    0, /* CTS Line: 0=not available, 1=available */
    0, /* DTR Line: 0=not available, 1=available */
    0, /* DSR Line: 0=not available, 1=available */
    0, /* DCD Line: 0=not available, 1=available */
    0, /* RI Line: 0=not available, 1=available */
    0, /* Signal CTS change event: \ref ARM_USART_EVENT_CTS */
    0, /* Signal DSR change event: \ref ARM_USART_EVENT_DSR */
    0, /* Signal DCD change event: \ref ARM_USART_EVENT_DCD */
    0, /* Signal RI change event: \ref ARM_USART_EVENT_RI */
    0  /* Reserved (must be zero) */
};

#define USART_NUM 3

#define PORTA_PIN9 9
#define PORTA_PIN10 10

#define PORTA_PIN2 2
#define PORTA_PIN3 3

#define PORTB_PIN10 26
#define PORTB_PIN11 27

#define USART1_TX PORTA_PIN9
#define USART1_RX PORTA_PIN10

#define USART2_TX PORTA_PIN2
#define USART2_RX PORTA_PIN3

#define USART3_TX PORTB_PIN10
#define USART3_RX PORTB_PIN11

extern ARM_DRIVER_GPIO Driver_GPIO0;

static inline void USART1_EnableClock(void);
static inline void USART2_EnableClock(void);
static inline void USART3_EnableClock(void);

typedef struct {
    USART_TypeDef* base;
    void (*enable_clock)(void);
    uint8_t tx_pin;
    uint8_t rx_pin;
    IRQn_Type irqn;
} USART_Config_t;

static const USART_Config_t usart_config[USART_NUM] = {{.base = USART1,
                                                        .enable_clock = USART1_EnableClock,
                                                        .tx_pin = USART1_TX,
                                                        .rx_pin = USART1_RX,
                                                        .irqn = USART1_IRQn},
                                                       {.base = USART2,
                                                        .enable_clock = USART2_EnableClock,
                                                        .tx_pin = USART2_TX,
                                                        .rx_pin = USART2_RX,
                                                        .irqn = USART2_IRQn},
                                                       {.base = USART3,
                                                        .enable_clock = USART3_EnableClock,
                                                        .tx_pin = USART3_TX,
                                                        .rx_pin = USART3_RX,
                                                        .irqn = USART3_IRQn}};

typedef struct {
    const USART_Config_t* cfg;
    ARM_USART_SignalEvent_t cb_event;

    volatile uint8_t initialized;
    volatile uint8_t powered;
    volatile uint32_t tx_count;
    volatile uint32_t rx_count;

    ARM_USART_STATUS status;
    ARM_USART_MODEM_STATUS modem_status;
} USART_Context_t;

static USART_Context_t usart_ctx[3];
static UART_TX_Buffer_t usart1_tx_queue;
static uint8_t usart1_tx_storage[256U];

/********************************************************************
 * Helper function
 ********************************************************************/
static int32_t USART_GetContext(uint8_t instance, USART_Context_t** ctx) {
    int32_t result = ARM_DRIVER_ERROR;

    if ((ctx != 0) && (instance >= 1U) && (instance <= 3U)) {
        *ctx = &usart_ctx[instance - 1];
        (*ctx)->cfg = &usart_config[instance - 1];
        result = ARM_DRIVER_OK;
    }

    return result;
}

static void USART_ResetContext(USART_Context_t* ctx) {
    ctx->cb_event = NULL;
    ctx->initialized = 0U;
    ctx->powered = 0U;
    ctx->tx_count = 0U;
    ctx->rx_count = 0U;

    ctx->status.tx_busy = 0U;
    ctx->status.rx_busy = 0U;
    ctx->status.tx_underflow = 0U;
    ctx->status.rx_overflow = 0U;
    ctx->status.rx_break = 0U;
    ctx->status.rx_framing_error = 0U;
    ctx->status.rx_parity_error = 0U;

    ctx->modem_status.cts = 0U;
    ctx->modem_status.dsr = 0U;
    ctx->modem_status.dcd = 0U;
    ctx->modem_status.ri = 0U;
}

static inline void USART1_EnableClock(void) {
    RCC_AFIO_CLK_EN();
    RCC_USART1_EN();
}

static inline void USART2_EnableClock(void) {
    RCC_AFIO_CLK_EN();
    RCC_USART2_EN();
}

static inline void USART3_EnableClock(void) {
    RCC_AFIO_CLK_EN();
    RCC_USART3_EN();
}

static void USART_GPIOInit(const USART_Config_t* cfg) {
    if (cfg == 0) {
        return;
    }

    Driver_GPIO0.Setup(cfg->tx_pin, NULL);
    Driver_GPIO0.SetDirection(cfg->tx_pin, ARM_GPIO_AF_OUTPUT);
    Driver_GPIO0.SetOutputMode(cfg->tx_pin, ARM_AFIO_PUSH_PULL);

    Driver_GPIO0.Setup(cfg->rx_pin, ARM_GPIO_INPUT);
    Driver_GPIO0.SetDirection(cfg->rx_pin, ARM_GPIO_INPUT);
    Driver_GPIO0.SetPullResistor(cfg->rx_pin, ARM_GPIO_PULL_UP);
}

static void USART1_TxKick(void) {
    USART1->CR1 |= (1U << USART_CR1_TXEIE_Pos);
}

/********************************************************************
 * USART Driver Function
 ********************************************************************/
static ARM_DRIVER_VERSION ARM_USART_GetVersion(void) {
    return DriverVersion;
}

static ARM_USART_CAPABILITIES ARM_USART_GetCapabilities(void) {
    return DriverCapabilities;
}

static int32_t ARM_USART1_Initialize(ARM_USART_SignalEvent_t cb_event) {
    int32_t result = ARM_DRIVER_OK;
    USART_Context_t* ctx;

    if (USART_GetContext(1, &ctx) != ARM_DRIVER_OK) {
        return ARM_DRIVER_ERROR;
    }
    USART_ResetContext(ctx);
    uart_tx_buffer_init(&usart1_tx_queue, usart1_tx_storage, (uint16_t)sizeof(usart1_tx_storage));

    ctx->cb_event = cb_event;
    ctx->initialized = 1;

    return result;
}

static int32_t ARM_USART1_Uninitialize(void) {
    int32_t result = ARM_DRIVER_OK;
    USART_Context_t* ctx;

    if (USART_GetContext(1, &ctx) != ARM_DRIVER_OK) {
        return ARM_DRIVER_ERROR;
    }

    USART1->CR1 &=
        ~((1U << USART_CR1_TXEIE_Pos) | (1U << USART_CR1_RXNEIE_Pos) | (1U << USART_CR1_TE_Pos) |
          (1U << USART_CR1_RE_Pos) | (1U << USART_CR1_UE_Pos));
    NVIC_DisableIRQ(USART1_IRQn);
    USART_ResetContext(ctx);

    return result;
}

static int32_t ARM_USART1_PowerControl(ARM_POWER_STATE state) {
    int32_t result = ARM_DRIVER_OK;
    USART_Context_t* ctx;

    if (USART_GetContext(1, &ctx) != ARM_DRIVER_OK) {
        return ARM_DRIVER_ERROR;
    }

    switch (state) {
        case ARM_POWER_OFF:
            USART1->CR1 &=
                ~((1U << USART_CR1_TXEIE_Pos) | (1U << USART_CR1_RXNEIE_Pos) |
                  (1U << USART_CR1_TE_Pos) | (1U << USART_CR1_RE_Pos) | (1U << USART_CR1_UE_Pos));
            NVIC_DisableIRQ(USART1_IRQn);
            ctx->powered = 0;
            break;

        case ARM_POWER_LOW:
            result = ARM_DRIVER_ERROR_UNSUPPORTED;
            break;

        case ARM_POWER_FULL:
            ctx->cfg->enable_clock();
            USART_GPIOInit(ctx->cfg);
            NVIC_SetPriority(ctx->cfg->irqn, 5);
            NVIC_EnableIRQ(ctx->cfg->irqn);
            ctx->powered = 1;
            break;

        default:
            result = ARM_DRIVER_ERROR;
            break;
    }
    return result;
}

static int32_t ARM_USART1_Send(const void* data, uint32_t num) {
    const uint8_t* source;
    uint32_t index;
    USART_Context_t* ctx;

    if (USART_GetContext(1U, &ctx) != ARM_DRIVER_OK) {
        return ARM_DRIVER_ERROR;
    }

    if ((ctx->powered == 0U) || (ctx->initialized == 0U)) {
        return ARM_DRIVER_ERROR;
    }

    if (num == 0U) {
        return ARM_DRIVER_OK;
    }

    if (data == 0) {
        return ARM_DRIVER_ERROR_PARAMETER;
    }

    source = (const uint8_t*)data;
    for (index = 0U; index < num; index++) {
        while (!uart_tx_buffer_put(&usart1_tx_queue, source[index])) {
            USART1_TxKick();
        }
    }

    ctx->tx_count += num;
    ctx->status.tx_busy = 1U;
    USART1_TxKick();

    return ARM_DRIVER_OK;
}

static int32_t ARM_USART1_Receive(void* data, uint32_t num) {}

static int32_t ARM_USART1_Transfer(const void* data_out, void* data_in, uint32_t num) {}

static uint32_t ARM_USART1_GetTxCount(void) {
    USART_Context_t* ctx;

    if (USART_GetContext(1U, &ctx) != ARM_DRIVER_OK) {
        return 0U;
    }

    return ctx->tx_count;
}

static uint32_t ARM_USART1_GetRxCount(void) {
    USART_Context_t* ctx;

    if (USART_GetContext(1U, &ctx) != ARM_DRIVER_OK) {
        return 0U;
    }

    return ctx->rx_count;
}

static int32_t ARM_USART1_Control(uint32_t control, uint32_t arg) {
    USART_Context_t* ctx;
    uint32_t data_bits;
    uint32_t parity_bits;
    uint32_t brr;
    uint32_t pclk;

    if (USART_GetContext(1U, &ctx) != ARM_DRIVER_OK) {
        return ARM_DRIVER_ERROR_PARAMETER;
    }

    if (ctx->powered == 0U) {
        return ARM_DRIVER_ERROR;
    }

    pclk = SystemCoreClock; /* USART1 on APB2, prescaler = 1 in your clock setup */

    switch (control & ARM_USART_CONTROL_Msk) {
        case ARM_USART_CONTROL_TX:
            if (arg) {
                USART1->CR1 |= (1U << USART_CR1_TE_Pos);
            } else {
                USART1->CR1 &= ~(1U << USART_CR1_TE_Pos);
            }
            return ARM_DRIVER_OK;

        case ARM_USART_CONTROL_RX:
            if (arg) {
                USART1->CR1 |= (1U << USART_CR1_RE_Pos);
            } else {
                USART1->CR1 &= ~(1U << USART_CR1_RE_Pos);
            }
            return ARM_DRIVER_OK;

        case ARM_USART_ABORT_SEND:
        case ARM_USART_ABORT_RECEIVE:
            return ARM_DRIVER_OK;

        case ARM_USART_MODE_ASYNCHRONOUS:
            /* Proceed to configure Asynchronous mode */
            break;

        default:
            return ARM_USART_ERROR_MODE;
    }

    /* Baudrate */
    if (arg == 0U) {
        return ARM_USART_ERROR_BAUDRATE;
    }

    /* Disable USART first to safely configure */
    USART1->CR1 &= ~(1U << USART_CR1_UE_Pos);

    /* Simple BRR calc for oversampling by 16 */
    brr = (pclk + (arg / 2U)) / arg;
    USART1->BRR = brr;

    /* Clear frame format first */
    USART1->CR1 &=
        ~((1U << USART_CR1_M_Pos) | (1U << USART_CR1_PCE_Pos) | (1U << USART_CR1_PS_Pos));

    USART1->CR2 &= ~((3U << USART_CR2_STOP_Pos));

    data_bits = control & ARM_USART_DATA_BITS_Msk;
    parity_bits = control & ARM_USART_PARITY_Msk;

    /* Parity */
    switch (parity_bits) {
        case ARM_USART_PARITY_NONE:
            USART1->CR1 &= ~(1U << USART_CR1_PCE_Pos);
            USART1->CR1 &= ~(1U << USART_CR1_PS_Pos);
            if (data_bits == ARM_USART_DATA_BITS_8) {
                USART1->CR1 &= ~(1U << USART_CR1_M_Pos);
            } else if (data_bits == ARM_USART_DATA_BITS_9) {
                USART1->CR1 |= (1U << USART_CR1_M_Pos);
            } else {
                return ARM_USART_ERROR_DATA_BITS;
            }
            break;

        case ARM_USART_PARITY_EVEN:
            USART1->CR1 |= (1U << USART_CR1_PCE_Pos);
            USART1->CR1 &= ~(1U << USART_CR1_PS_Pos);
            if (data_bits == ARM_USART_DATA_BITS_8) {
                USART1->CR1 |= (1U << USART_CR1_M_Pos);
            } else if (data_bits == ARM_USART_DATA_BITS_7) {
                USART1->CR1 &= ~(1U << USART_CR1_M_Pos);
            } else {
                return ARM_USART_ERROR_DATA_BITS;
            }
            break;

        case ARM_USART_PARITY_ODD:
            USART1->CR1 |= (1U << USART_CR1_PCE_Pos);
            USART1->CR1 |= (1U << USART_CR1_PS_Pos);
            if (data_bits == ARM_USART_DATA_BITS_8) {
                USART1->CR1 |= (1U << USART_CR1_M_Pos);
            } else if (data_bits == ARM_USART_DATA_BITS_7) {
                USART1->CR1 &= ~(1U << USART_CR1_M_Pos);
            } else {
                return ARM_USART_ERROR_DATA_BITS;
            }
            break;

        default:
            return ARM_USART_ERROR_PARITY;
    }

    /* Stop bits */
    switch (control & ARM_USART_STOP_BITS_Msk) {
        case ARM_USART_STOP_BITS_1:
            USART1->CR2 |= (0U << USART_CR2_STOP_Pos);
            break;
        case ARM_USART_STOP_BITS_2:
            USART1->CR2 |= (2U << USART_CR2_STOP_Pos);
            break;
        case ARM_USART_STOP_BITS_1_5:
            USART1->CR2 |= (1U << USART_CR2_STOP_Pos);
            break;
        case ARM_USART_STOP_BITS_0_5:
            USART1->CR2 |= (3U << USART_CR2_STOP_Pos);
            break;
        default:
            return ARM_USART_ERROR_STOP_BITS;
    }

    /* Enable USART */
    USART1->CR1 |= (1U << USART_CR1_UE_Pos);

    return ARM_DRIVER_OK;
}

static ARM_USART_STATUS ARM_USART1_GetStatus(void) {
    USART_Context_t* ctx;

    if (USART_GetContext(1U, &ctx) != ARM_DRIVER_OK) {
        ARM_USART_STATUS empty_status = {0};
        return empty_status;
    }

    return ctx->status;
}

static int32_t ARM_USART1_SetModemControl(ARM_USART_MODEM_CONTROL control) {
    (void)control;
    return ARM_DRIVER_ERROR_UNSUPPORTED;
}

static ARM_USART_MODEM_STATUS ARM_USART1_GetModemStatus(void) {
    USART_Context_t* ctx;
    ARM_USART_MODEM_STATUS empty_status = {0};

    if (USART_GetContext(1U, &ctx) != ARM_DRIVER_OK) {
        return empty_status;
    }

    return ctx->modem_status;
}

static void ARM_USART1_SignalEvent(uint32_t event) {
    // function body
}

void USART1_IRQHandler(void) {
    USART_Context_t* ctx;
    uint8_t data;

    if (USART_GetContext(1U, &ctx) != ARM_DRIVER_OK) {
        return;
    }

    if (((USART1->SR & USART_SR_TXE) != 0U) && ((USART1->CR1 & (1U << USART_CR1_TXEIE_Pos)) != 0U)) {
        if (uart_tx_buffer_get(&usart1_tx_queue, &data)) {
            USART1->DR = data;
        } else {
            USART1->CR1 &= ~(1U << USART_CR1_TXEIE_Pos);
            ctx->status.tx_busy = 0U;
        }
    }
}

// End USART Interface

extern ARM_DRIVER_USART Driver_USART1;
ARM_DRIVER_USART Driver_USART1 = {
    ARM_USART_GetVersion,       ARM_USART_GetCapabilities, ARM_USART1_Initialize,
    ARM_USART1_Uninitialize,    ARM_USART1_PowerControl,   ARM_USART1_Send,
    ARM_USART1_Receive,         ARM_USART1_Transfer,       ARM_USART1_GetTxCount,
    ARM_USART1_GetRxCount,      ARM_USART1_Control,        ARM_USART1_GetStatus,
    ARM_USART1_SetModemControl, ARM_USART1_GetModemStatus};
