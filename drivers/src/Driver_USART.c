/**
 * @file    Driver_USART.c
 * @author  DinhTien (tien.ta.eswe@gmail.com)
 * @brief   USART Peripheral Driver implementation following CMSIS-Driver standard.
 * @details Implements asynchronous UART transmission for USART1 peripheral on 
 * STM32F103 MCU. Integrates an interrupt-driven non-blocking Ring Buffer 
 * mechanism for efficient transmission execution without CPU stalling.
 * @version 1.0
 * @date    2026-05-17
 *
 * @copyright Copyright (c) 2026
 *
 */

#include <string.h>

#include "Driver_USART.h"
#include "Driver_GPIO.h"
#include "Driver_RCC.h"

#include "bsp_config.h"
#include "ring_buffer.h"

#include "stm32f103xb.h"

/********************************************************************
 * Definitions
 ********************************************************************/

#define ARM_USART_DRV_VERSION    ARM_DRIVER_VERSION_MAJOR_MINOR(1, 0)  /**< Driver implementation version. */
#define BUFFER_SIZE              256                                   /**< Ring buffer sizing window, must be power of 2. */

/**
 * @brief  Internal state monitoring layout context tracking for the active USART.
 */
typedef struct {
    volatile uint8_t initialized;       /**< Initialization registration verification flag. */
    volatile uint8_t powered;           /**< Peripheral master power state tracking flag. */
    volatile uint32_t tx_count;         /**< Rolling total counters of bytes pushed successfully onto TX wires. */
    volatile uint32_t rx_count;         /**< Rolling total counters of bytes collected from RX wires. */
    volatile uint32_t controlled;       /**< Control register state execution confirmation check. */
    ARM_USART_STATUS status;            /**< Current runtime peripheral operational status flags. */
    ARM_USART_MODEM_STATUS modem_status;/**< Hardware modem line pin signals state tracking parameters. */
} USART_Context_t;

/* Driver Version */
static const ARM_DRIVER_VERSION DriverVersion = { 
    ARM_USART_API_VERSION,
    ARM_USART_DRV_VERSION
};

/* Driver Capabilities Definition Structure */
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

/*******************************************************************************
 * Prototypes
 ******************************************************************************/
static void USART1_ResetContext(USART_Context_t *ctx);
static ARM_DRIVER_VERSION ARM_USART_GetVersion(void);
static int32_t ARM_USART_Initialize(ARM_USART_SignalEvent_t cb_event);
static int32_t ARM_USART_Uninitialize(void);
static int32_t ARM_USART_PowerControl(ARM_POWER_STATE state);
static int32_t ARM_USART_Send(const void *data, uint32_t num);
static int32_t ARM_USART_Receive(void *data, uint32_t num);
static int32_t ARM_USART_Transfer(const void *data_out, void *data_in, uint32_t num);
static uint32_t ARM_USART_GetTxCount(void);
static uint32_t ARM_USART_GetRxCount(void);
static int32_t ARM_USART_Control(uint32_t control, uint32_t arg);
static ARM_USART_STATUS ARM_USART_GetStatus(void);
static int32_t ARM_USART_SetModemControl(ARM_USART_MODEM_CONTROL control);
static ARM_USART_MODEM_STATUS ARM_USART_GetModemStatus(void);

/*******************************************************************************
 * Variables
 ******************************************************************************/

static USART_Context_t usart1_ctx;                    /**< Core internal runtime state variable layout space for USART1. */
static uint8_t usart1_tx_data[BUFFER_SIZE];          /**< Raw data storage block allocating byte capacity for ring buffer. */
static ring_buffer_t usart1_tx_rb;                    /**< Active Ring Buffer tracking context reference structure. */
static ARM_USART_SignalEvent_t cb_ev;                 /**< App space global upper level event callback function dispatcher. */

extern ARM_DRIVER_GPIO Driver_GPIO0;

/*******************************************************************************
 * Code
 ******************************************************************************/

/**
 * @brief  Clears out data layouts inside operational context workspace structure blocks.
 * @param  ctx: Pointer to structural context memory target to erase.
 * @return None
 */
static void USART1_ResetContext(USART_Context_t *ctx) {
    if (ctx != NULL) {
        memset(ctx, 0, sizeof(USART_Context_t));
    }
}

/**
 * @brief  Fetches CMSIS API specification compatibility level versions.
 * @return ARM_DRIVER_VERSION: Active driver version schema parameters.
 */
static ARM_DRIVER_VERSION ARM_USART_GetVersion(void)
{
  return DriverVersion;
}

/**
 * @brief  Inquires specific architectural capability flags bounded by driver layouts.
 * @return ARM_USART_CAPABILITIES: Bit fields indicating explicit peripheral configurations.
 */
static ARM_USART_CAPABILITIES ARM_USART_GetCapabilities(void)
{
  return DriverCapabilities;
}

/**
 * @brief  Initializes context variables, creates safe Ring Buffer blocks, and binds callbacks.
 * @param  cb_event: Operational upper level runtime callback function signal pointer target.
 * @retval ARM_DRIVER_OK: Memory and circular bounds mapped safely.
 * @retval ARM_DRIVER_ERROR: Passed null parameters or buffer initiation failure hooks.
 */
static int32_t ARM_USART_Initialize(ARM_USART_SignalEvent_t cb_event)
{
    if (cb_event == NULL) {
        return ARM_DRIVER_ERROR;
    }

    USART1_ResetContext(&usart1_ctx);
    usart1_ctx.initialized = 1;

    if (!rb_init(&usart1_tx_rb, usart1_tx_data, BUFFER_SIZE)) {
        return ARM_DRIVER_ERROR;
    }
    cb_ev = cb_event;

    return ARM_DRIVER_OK;
}

/**
 * @brief  Dismantles and releases driver status contexts back into default states.
 * @retval ARM_DRIVER_OK: De-allocation or baseline resets completed successfully.
 */
static int32_t ARM_USART_Uninitialize(void)
{
    USART1_ResetContext(&usart1_ctx);

    return ARM_DRIVER_OK;
}

/**
 * @brief  Controls energy states by triggering internal clock gating and mapping IO lines.
 * @param  state: Desired dynamic power constraint layout setting profile (@ref ARM_POWER_STATE).
 * @retval ARM_DRIVER_OK: Target power mode configurations written successfully.
 * @retval ARM_DRIVER_ERROR_UNSUPPORTED: Triggered unhandled execution profiles like low power sleep.
 */
static int32_t ARM_USART_PowerControl(ARM_POWER_STATE state)
{
    int32_t result = ARM_DRIVER_OK;

    switch (state)
    {
    case ARM_POWER_OFF:
        /* Unmask control bits and cut off clock engines */
        USART1->CR1 &= ~((1 << USART_CR1_UE_Pos) | (1 << USART_CR1_RE_Pos) | (1 << USART_CR1_TE_Pos));
        usart1_ctx.powered = 0;
        NVIC_DisableIRQ(USART1_IRQn);
        break;

    case ARM_POWER_LOW:
        result = ARM_DRIVER_ERROR_UNSUPPORTED;
        break;

    case ARM_POWER_FULL:
        /* Route peripheral engine clock triggers */
        RCC_AFIO_CLK_EN();
        RCC_USART1_EN();
        
        /* Map Alternate Function push-pull layout to native TX GPIO pin hardware line */
        Driver_GPIO0.Setup(USART1_TX_PIN, NULL);
        Driver_GPIO0.SetDirection(USART1_TX_PIN, ARM_GPIO_AF_OUTPUT);
        Driver_GPIO0.SetOutputMode(USART1_TX_PIN, ARM_AFIO_PUSH_PULL);
        
        /* Prioritize and unleash interrupt controllers inside core NVIC */
        NVIC_SetPriority(USART1_IRQn, 5);
        NVIC_EnableIRQ(USART1_IRQn);
        usart1_ctx.powered = 1;
        break;
    }
    return result;
}

/**
 * @brief  Pushes target message packets into internal dynamic Ring Buffer queues to ignite transmission.
 * @note   Triggers Transmit Data Register Empty Interrupt (TXEIE) to execute thread safe offloading.
 * @param  data: Pointer pointing to the source data stream array block to transmit.
 * @param  num: Exact absolute counts of message bytes to inject into target pipelines.
 * @return int32_t: Returns total length integer of bytes cached successfully, or error flags.
 */
static int32_t ARM_USART_Send(const void *data, uint32_t num)
{
    if (data == NULL || num == 0) {
        return ARM_DRIVER_ERROR;
    }

    if (usart1_ctx.initialized == 0 || usart1_ctx.powered == 0 || usart1_ctx.controlled == 0) {
        return ARM_DRIVER_ERROR;
    }

    uint8_t *p_data = (uint8_t *)data;
    uint16_t byte_copied = 0;

    /* Populate ring buffer slots without thread locking operations */
    while (byte_copied < num) {
        if (!rb_put(&usart1_tx_rb, *p_data++)) {
            break; /* Buffer allocation overflow floor ceiling reached */
        } else {
            byte_copied++;
        }
    }

    /* Ignite transactional data line shifting by engaging TXE interrupts */
    if (byte_copied > 0) {
        USART1->CR1 |= (1 << USART_CR1_TXEIE_Pos);
    }

    return byte_copied;
}

/**
 * @brief  Receives data from USART receiver. (Not implemented).
 */
static int32_t ARM_USART_Receive(void *data, uint32_t num)
{
    (void)data;
    (void)num;
    return ARM_DRIVER_ERROR_UNSUPPORTED;
}

/**
 * @brief  Executes synchronous duplex stream parsing operations. (Not supported in async).
 */
static int32_t ARM_USART_Transfer(const void *data_out, void *data_in, uint32_t num)
{
    (void)data_out;
    (void)data_in;
    (void)num;

    return ARM_DRIVER_ERROR_UNSUPPORTED;
}

/**
 * @brief  Reads active absolute byte counts successfully processed out via TX structures.
 * @return uint32_t: Accumulative totals.
 */
static uint32_t ARM_USART_GetTxCount(void)
{
    return usart1_ctx.tx_count;
}

/**
 * @brief  Reads active absolute byte counts parsed in via RX structures.
 * @return uint32_t: Accumulative totals.
 */
static uint32_t ARM_USART_GetRxCount(void)
{
    return usart1_ctx.rx_count;
}

/**
 * @brief  Configures protocol constraints, overrides frame configurations, and sets Baudrate ratios.
 * @param  control: Operation control configuration masked flags parameter (@ref USART_control_flags).
 * @param  arg: Multi-purpose numerical parameter representing explicitly target Baudrate speed parameters.
 * @retval ARM_DRIVER_OK: Protocol properties modified on active hardware blocks successfully.
 * @retval ARM_DRIVER_ERROR: Out of bounds arguments or configuration violations.
 */
static int32_t ARM_USART_Control(uint32_t control, uint32_t arg)
{
    if (usart1_ctx.initialized == 0 || usart1_ctx.powered == 0) {
        return ARM_DRIVER_ERROR;
    }
    switch (control & ARM_USART_CONTROL_Msk) {
        case ARM_USART_MODE_ASYNCHRONOUS: {
            if (arg <= 0) {
                return ARM_DRIVER_ERROR;
            }

            uint32_t pclk = 72000000; /* USART1 is hooked straight into high-speed APB2 clock tree line */
            uint16_t brr_val;

            /* Extract Data Word Frame Bit Widths Length fields */
            switch(control & ARM_USART_DATA_BITS_Msk) {
                case ARM_USART_DATA_BITS_8:
                    USART1->CR1 &= ~(1 << USART_CR1_M_Pos);
                    break;
                case ARM_USART_DATA_BITS_9:
                    USART1->CR1 |= (1 << USART_CR1_M_Pos);
                    break;
                default:
                    return ARM_DRIVER_ERROR;
            }

            /* Extract and apply explicit Parity confirmation block mappings */
            switch (control & ARM_USART_PARITY_Msk) {
                case ARM_USART_PARITY_NONE:
                    USART1->CR1 &= ~(1 << USART_CR1_PCE_Pos);
                    break;
                case ARM_USART_PARITY_EVEN:
                    USART1->CR1 &= ~(1 << USART_CR1_PS_Pos);
                    USART1->CR1 |= (1 << USART_CR1_PCE_Pos);
                    break;
                case ARM_USART_PARITY_ODD:
                    USART1->CR1 |= (1 << USART_CR1_PS_Pos);
                    USART1->CR1 |= (1 << USART_CR1_PCE_Pos);
                    break;
                default:
                    return ARM_DRIVER_ERROR;
            }

            /* Map physical framing boundary Stop Bits configurations */
            switch (control & ARM_USART_STOP_BITS_Msk) {
                case ARM_USART_STOP_BITS_1:
                    USART1->CR2 &= ~(0b11 << USART_CR2_STOP_Pos);
                    break;
                case ARM_USART_STOP_BITS_2:
                    USART1->CR2 &= ~(0b11 << USART_CR2_STOP_Pos);
                    USART1->CR2 |= (0b10 << USART_CR2_STOP_Pos);
                    break;
                case ARM_USART_STOP_BITS_1_5:
                    USART1->CR2 &= ~(0b11 << USART_CR2_STOP_Pos);
                    USART1->CR2 |= (0b11 << USART_CR2_STOP_Pos);
                    break;
                case ARM_USART_STOP_BITS_0_5:
                    USART1->CR2 &= ~(0b11 << USART_CR2_STOP_Pos);
                    USART1->CR2 |= (0b01 << USART_CR2_STOP_Pos); /* Fixed mask literal syntax glitch */
                    break;
                default:
                    return ARM_DRIVER_ERROR;
            }

            /* Calculate Baudrate Generator fractional division scales: BRR = PCLK / Baudrate */
            brr_val = (pclk + (arg / 2)) / arg;
            USART1->BRR = brr_val;
            break;
        }
        case ARM_USART_CONTROL_TX:
            if (arg == 0) {
                USART1->CR1 &= ~(1 << USART_CR1_TE_Pos);
            } else {
                USART1->CR1 |= (1 << USART_CR1_TE_Pos);
            }
            break;

        case ARM_USART_CONTROL_RX:
            return ARM_DRIVER_ERROR_UNSUPPORTED;
        default:
            return ARM_DRIVER_ERROR_UNSUPPORTED;
    }

    /* Engage Master USART Enable (UE) bit to activate hardware blocks */
    USART1->CR1 |= (1 << USART_CR1_UE_Pos);
    usart1_ctx.controlled = 1;

    return ARM_DRIVER_OK;
}

/**
 * @brief  Reads external public status structure states.
 * @return ARM_USART_STATUS: Mask status flags indicator properties.
 */
static ARM_USART_STATUS ARM_USART_GetStatus(void)
{
    return usart1_ctx.status;
}

/**
 * @brief  Configures hardware modem signal lines. (Not supported).
 */
static int32_t ARM_USART_SetModemControl(ARM_USART_MODEM_CONTROL control)
{
    (void) control;
    return ARM_DRIVER_ERROR_UNSUPPORTED;
}

/**
 * @brief  Reads active hardware modem electrical line pin configurations.
 * @return ARM_USART_MODEM_STATUS: Structure output variables.
 */
static ARM_USART_MODEM_STATUS ARM_USART_GetModemStatus(void)
{
    return usart1_ctx.modem_status;
}

__attribute__((unused)) static void ARM_USART_SignalEvent(uint32_t event)
{
    (void)event;
}

/**
 * @brief  Interrupt Service Routine (ISR) capturing hardware USART1 signals.
 * @details Seamlessly routes character bytes out from active Ring Buffer memory queues 
 * onto physical output pipelines whenever TXE is tripped. Dispatches standard 
 * complete triggers (`ARM_USART_EVENT_TX_COMPLETE`) through callback pipelines 
 * once shifting pipelines are empty (TC).
 * @return None
 */
void USART1_IRQHandler(void) {
    uint32_t sr = USART1->SR;
    uint32_t cr1 = USART1->CR1;

    /* Transmit Data Register Empty Interrupt Handling */
    if ((sr & USART_SR_TXE) && (cr1 & USART_CR1_TXEIE)) {
        uint8_t data;
        if (rb_get(&usart1_tx_rb, &data)) {
            USART1->DR = data;
        }
        else {
            /* No more data left inside the buffer; switch off TXEIE and verify line clearance via TCIE */
            USART1->CR1 &= ~(1 << USART_CR1_TXEIE_Pos);
            USART1->CR1 |= (1 << USART_CR1_TCIE_Pos);
        }
    }

    /* Transmission Complete Interrupt Handling */
    if ((sr & USART_SR_TC) && (cr1 & USART_CR1_TCIE)) {
        /* Clear trigger masks to secure system bounds */
        USART1->CR1 &= ~(1 << USART_CR1_TCIE_Pos);
        USART1->SR &= ~(1 << USART_SR_TC_Pos);

        /* Signal transaction completion event to upper application layer */
        if (cb_ev != NULL) {
            cb_ev(ARM_USART_EVENT_TX_COMPLETE);
        }
    }
}

/**
 * @brief Global structure instance for binding USART Driver capabilities.
 */
ARM_DRIVER_USART Driver_USART1 = {
    ARM_USART_GetVersion,
    ARM_USART_GetCapabilities,
    ARM_USART_Initialize,
    ARM_USART_Uninitialize,
    ARM_USART_PowerControl,
    ARM_USART_Send,
    ARM_USART_Receive,
    ARM_USART_Transfer,
    ARM_USART_GetTxCount,
    ARM_USART_GetRxCount,
    ARM_USART_Control,
    ARM_USART_GetStatus,
    ARM_USART_SetModemControl,
    ARM_USART_GetModemStatus
};
