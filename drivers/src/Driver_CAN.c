/**
 * @file    Driver_CAN.c
 * @author  DinhTien (tien.ta.eswe@gmail.com)
 * @brief   bxCAN Peripheral Driver implementation following CMSIS-Driver standard.
 * @details Manages the basic extended CAN controller (bxCAN) for STM32F103 MCU. 
 * Maps Object Indexes 0-2 straight to physical Transmit Mailboxes, and Object 
 * Index 3 to Receive FIFO 0. Supports dynamic filter activation, standard/extended 
 * frame parsing, and secure interrupt integration with RTOS kernel priorities.
 * @version 1.0
 * @date    2026-05-18
 *
 * @copyright Copyright (c) 2026
 *
 */
 
#include "Driver_CAN.h"
#include "Driver_RCC.h"
#include "Driver_GPIO.h"

#include "bsp_config.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/

#define ARM_CAN_DRV_VERSION ARM_DRIVER_VERSION_MAJOR_MINOR(1,0) /**< CAN driver implementation version. */

/* Driver Version Static Constant Binding */
static const ARM_DRIVER_VERSION can_driver_version = { ARM_CAN_API_VERSION, ARM_CAN_DRV_VERSION };

/* Driver Capabilities Profile Specification Mapping */
static const ARM_CAN_CAPABILITIES can_driver_capabilities = {
  32U,  /* Number of CAN Objects available (Allocated software channels bound) */
  0U,   /* Does not support reentrant calls to core communication functions */
  0U,   /* Does not support CAN with Flexible Data-rate mode (CAN_FD limitation on F103) */
  0U,   /* Does not support restricted operation mode */
  0U,   /* Does not support bus monitoring mode */
  0U,   /* Does not support internal loopback mode */
  0U,   /* Does not support external loopback mode */
  0U    /* Reserved (must be zero) */
};

/* Object Channel Capabilities Profile Map */
static const ARM_CAN_OBJ_CAPABILITIES can_object_capabilities = {
  1U,   /* Object supports transmission */
  1U,   /* Object supports reception */
  0U,   /* Object does not support RTR reception and automatic Data transmission */
  0U,   /* Object does not support RTR transmission and automatic Data reception */
  0U,   /* Object does not allow assignment of multiple filters to it */
  0U,   /* Object does not support exact identifier filtering */
  0U,   /* Object does not support range identifier filtering */
  0U,   /* Object does not support mask identifier filtering */
  0U,   /* Object can not buffer messages */
  0U    /* Reserved (must be zero) */
};

/*******************************************************************************
 * Prototypes
 ******************************************************************************/

static ARM_DRIVER_VERSION ARM_CAN_GetVersion (void);
static ARM_CAN_CAPABILITIES ARM_CAN_GetCapabilities (void);
static int32_t ARM_CAN_Initialize (ARM_CAN_SignalUnitEvent_t   cb_unit_event,
                                   ARM_CAN_SignalObjectEvent_t cb_object_event);
static int32_t ARM_CAN_Uninitialize (void);
static int32_t ARM_CAN_PowerControl (ARM_POWER_STATE state);
static uint32_t ARM_CAN_GetClock (void);
static int32_t ARM_CAN_SetBitrate (ARM_CAN_BITRATE_SELECT select, uint32_t bitrate, uint32_t bit_segments);
static int32_t ARM_CAN_SetMode (ARM_CAN_MODE mode);
static ARM_CAN_OBJ_CAPABILITIES ARM_CAN_ObjectGetCapabilities (uint32_t obj_idx);
static int32_t ARM_CAN_ObjectSetFilter (uint32_t obj_idx, ARM_CAN_FILTER_OPERATION operation, uint32_t id, uint32_t arg);
static int32_t ARM_CAN_ObjectConfigure (uint32_t obj_idx, ARM_CAN_OBJ_CONFIG obj_cfg);
static int32_t ARM_CAN_MessageSend (uint32_t obj_idx, ARM_CAN_MSG_INFO *msg_info, const uint8_t *data, uint8_t size);
static int32_t ARM_CAN_MessageRead (uint32_t obj_idx, ARM_CAN_MSG_INFO *msg_info, uint8_t *data, uint8_t size);
static int32_t ARM_CAN_Control (uint32_t control, uint32_t arg);
static ARM_CAN_STATUS ARM_CAN_GetStatus (void);

/*******************************************************************************
 * Variables
 ******************************************************************************/

static uint8_t                     can_driver_powered     = 0U; /**< Runtime monitor tracking peripheral power state. */
static uint8_t                     can_driver_initialized = 0U; /**< Initialization tracking lock flag. */
static ARM_CAN_SignalUnitEvent_t   CAN_SignalUnitEvent    = NULL; /**< Core unit wide system status global callback context pointer. */
static ARM_CAN_SignalObjectEvent_t CAN_SignalObjectEvent  = NULL; /**< Individual channel message event callback dispatcher. */

extern ARM_DRIVER_GPIO Driver_GPIO0;

/*******************************************************************************
 * Code
 ******************************************************************************/

/**
 * @brief  Fetches CMSIS API interface compatibility version structures.
 * @return ARM_DRIVER_VERSION: Driver specification schema parameters.
 */
static ARM_DRIVER_VERSION ARM_CAN_GetVersion (void) {
  return can_driver_version;
}

/**
 * @brief  Inquires master technical structural capability bitmasks bounded by driver.
 * @return ARM_CAN_CAPABILITIES: Capabilities configuration structures.
 */
static ARM_CAN_CAPABILITIES ARM_CAN_GetCapabilities (void) {
  /* Return driver capabilities structural constant defined at the top of this file */
  return can_driver_capabilities;
}

/**
 * @brief  Initializes IO lines, sets routing configurations, and anchors operational callbacks.
 * @param  cb_unit_event: Global peripheral architecture status signal callback target.
 * @param  cb_object_event: Extracted transactional payload event notification callback function.
 * @retval ARM_DRIVER_OK: Safe clock and pin mapping complete.
 */
static int32_t ARM_CAN_Initialize (ARM_CAN_SignalUnitEvent_t   cb_unit_event,
                                   ARM_CAN_SignalObjectEvent_t cb_object_event) {

  if (can_driver_initialized != 0U) { return ARM_DRIVER_OK; }

  CAN_SignalUnitEvent   = cb_unit_event;
  CAN_SignalObjectEvent = cb_object_event;

  /* Enable peripheral core gating clock blocks */
  RCC_AFIO_CLK_EN();
  RCC_GPIOA_CLK_EN();

  /* Route Alternate Function push-pull topology to native physical CAN_TX pin */
  Driver_GPIO0.Setup(CAN_TX_PIN, NULL);
  Driver_GPIO0.SetDirection(CAN_TX_PIN, ARM_GPIO_AF_OUTPUT);
  Driver_GPIO0.SetOutputMode(CAN_TX_PIN, ARM_AFIO_PUSH_PULL);

  /* Configure safe input impedance constraints onto physical CAN_RX pin */
  Driver_GPIO0.Setup(CAN_RX_PIN, NULL);
  Driver_GPIO0.SetDirection(CAN_RX_PIN, ARM_GPIO_INPUT);
  can_driver_initialized = 1U;

  return ARM_DRIVER_OK;
}

/**
 * @brief  Releases active initialization states and drops hardware control locks.
 * @retval ARM_DRIVER_OK: Driver state dropped.
 */
static int32_t ARM_CAN_Uninitialize (void) {
  can_driver_initialized = 0U;
  return ARM_DRIVER_OK;
}

/**
 * @brief  Manages peripheral core master power gating states and handles NVIC setups.
 * @note   Interrupt lines are prioritized at level 5 to secure safe execution inside FreeRTOS.
 * @param  state: Selected operational target system power restriction constraint (@ref ARM_POWER_STATE).
 * @retval ARM_DRIVER_OK: Peripheral status changed cleanly.
 * @retval ARM_DRIVER_ERROR: Initialization locks missing during active boot attempt.
 * @retval ARM_DRIVER_ERROR_UNSUPPORTED: Unsupported low power sleep invocation profiles.
 */
static int32_t ARM_CAN_PowerControl (ARM_POWER_STATE state) {
  switch (state) {
    case ARM_POWER_OFF:
      can_driver_powered = 0U;

      /* Strip NVIC unmasks and drop register structures into clear states */
      NVIC_DisableIRQ(USB_HP_CAN1_TX_IRQn);
      NVIC_DisableIRQ(USB_LP_CAN1_RX0_IRQn);
      RCC->APB1RSTR |= RCC_APB1RSTR_CAN1RST;
      RCC->APB1RSTR &= ~RCC_APB1RSTR_CAN1RST;
      RCC->APB1ENR &= ~RCC_APB1ENR_CAN1EN;
      break;

    case ARM_POWER_FULL:
      if (can_driver_initialized == 0U) { return ARM_DRIVER_ERROR; }
      if (can_driver_powered     != 0U) { return ARM_DRIVER_OK;    }

      /* Feed operational peripheral clock bus */
      RCC->APB1ENR |= RCC_APB1ENR_CAN1EN;
      
      /* Issue Initialisation Request (INRQ) to enter setup mode window */
      CAN1->MCR |= CAN_MCR_INRQ;
      while ((CAN1->MSR & CAN_MSR_INAK) == 0);
      
      /* Release controller hardware out from Sleep Mode */
      CAN1->MCR &= ~CAN_MCR_SLEEP;
      
      /* Establish core Interrupt Masks (FIFO0 Message Pending and Transmit Mailbox Empty) */
      NVIC_ClearPendingIRQ(USB_HP_CAN1_TX_IRQn);
      CAN1->IER |= CAN_IER_FMPIE0 | CAN_IER_TMEIE;
      
      /* Secure RTOS nesting bounds: Priorities must be >= configMAX_SYSCALL_INTERRUPT_PRIORITY (5) */
      NVIC_SetPriority(USB_HP_CAN1_TX_IRQn, 5);
      NVIC_SetPriority(USB_LP_CAN1_RX0_IRQn, 5);
      
      NVIC_EnableIRQ(USB_HP_CAN1_TX_IRQn);
      NVIC_ClearPendingIRQ(USB_LP_CAN1_RX0_IRQn);
      NVIC_EnableIRQ(USB_LP_CAN1_RX0_IRQn);
      can_driver_powered = 1U;
      break;

    case ARM_POWER_LOW:
      return ARM_DRIVER_ERROR_UNSUPPORTED;
  }

  return ARM_DRIVER_OK;
}

/**
 * @brief  Reads baseline source execution peripheral feeding clock speed.
 * @return uint32_t: Core base operating speed value in Hertz.
 */
static uint32_t ARM_CAN_GetClock (void) {
  return ARM_DRIVER_OK;
}

/**
 * @brief  Configures protocol speed metrics and programs the physical CAN Bit Timing Register (BTR).
 * @note   Calculations are targeted for 500kbps rate over a 36MHz peripheral APB1 bus line.
 * Total Bit Time = 1 + TS1 + TS2 = 1 + 14 + 3 = 18 Time Quanta (tq).
 * Baudrate = PCLK / (BRP * Total Bit Time) = 36MHz / (4 * 18) = 500 kbps.
 * @param  select: Parameter speed type index profile selection filter (@ref ARM_CAN_BITRATE_SELECT).
 * @param  bitrate: Absolute execution speed targeted on bus lines (Strictly enforces 500000 bps).
 * @param  bit_segments: Specific segmentation properties. (Handled via native manual hardware presets).
 * @retval ARM_DRIVER_OK: Nominal time register bits configured correctly.
 * @retval ARM_DRIVER_ERROR: Power verification lock missing.
 * @retval ARM_DRIVER_ERROR_UNSUPPORTED: Requested unsupported bus speeds.
 */
static int32_t ARM_CAN_SetBitrate (ARM_CAN_BITRATE_SELECT select, uint32_t bitrate, uint32_t bit_segments) {

  if (can_driver_powered == 0U) { return ARM_DRIVER_ERROR; }

  if (select != ARM_CAN_BITRATE_NOMINAL) {
      return ARM_DRIVER_ERROR_UNSUPPORTED;
  }

  if (bitrate != 500000U) {
      return ARM_DRIVER_ERROR_UNSUPPORTED; 
  }

  /* Force entry into Initialization Mode to securely enable write access onto BTR block */
  if ((CAN1->MSR & CAN_MSR_INAK) == 0) {
      CAN1->MCR |= CAN_MCR_INRQ;
      while ((CAN1->MSR & CAN_MSR_INAK) == 0); 
  }

  /* * Timing Mappings:
   * SJW = 0 (1 tq)
   * TS2 = 2 (3 tq)
   * TS1 = 13 (14 tq)
   * BRP = 3 (Prescaler scale ratio factor = 4)
   */
  CAN1->BTR = (0U << 24) |    
              (2U << 20) |    
              (13U << 16) |   
              (3U << 0);      

  return ARM_DRIVER_OK;
}

/**
 * @brief  Switches structural test profiles and steps modes inside master control logs.
 * @param  mode: Selected mode index classification modifier profile (@ref ARM_CAN_MODE).
 * @retval ARM_DRIVER_OK: Operating behavior properties adjusted successfully.
 */
static int32_t ARM_CAN_SetMode (ARM_CAN_MODE mode) {

  if (can_driver_powered == 0U) { return ARM_DRIVER_ERROR; }

  switch (mode) {
    case ARM_CAN_MODE_INITIALIZATION:
      CAN1->MCR |= CAN_MCR_INRQ;
      while ((CAN1->MSR & CAN_MSR_INAK) == 0);
      break;
    case ARM_CAN_MODE_NORMAL:
      CAN1->MCR |= CAN_MCR_INRQ;
      while ((CAN1->MSR & CAN_MSR_INAK) == 0);
      /* Wipe diagnostic testing bits to execute true differential wire driving */
      CAN1->BTR &= ~(CAN_BTR_LBKM | CAN_BTR_SILM);
      
      /* Exit initialization mode to connect to active CAN network */
      CAN1->MCR &= ~CAN_MCR_INRQ;
      while ((CAN1->MSR & CAN_MSR_INAK) != 0);
      break;
    case ARM_CAN_MODE_RESTRICTED:
      break;
    case ARM_CAN_MODE_MONITOR:
      CAN1->MCR |= CAN_MCR_INRQ;
      while ((CAN1->MSR & CAN_MSR_INAK) == 0);

      /* Silent Mode: Receiver works normal, transmission logic is disconnected internally */
      CAN1->BTR &= ~CAN_BTR_LBKM;
      CAN1->BTR |= CAN_BTR_SILM;
      CAN1->MCR &= ~CAN_MCR_INRQ;
      while ((CAN1->MSR & CAN_MSR_INAK) != 0);
      break;
    case ARM_CAN_MODE_LOOPBACK_INTERNAL:
      CAN1->MCR |= CAN_MCR_INRQ;
      while ((CAN1->MSR & CAN_MSR_INAK) == 0);

      /* Loopback Mode: Diverts internal TX streams straight into core RX input structures */
      CAN1->BTR &= ~CAN_BTR_SILM;
      CAN1->BTR |= CAN_BTR_LBKM; 

      CAN1->MCR &= ~CAN_MCR_INRQ;
      while ((CAN1->MSR & CAN_MSR_INAK) != 0);
      break;
    case ARM_CAN_MODE_LOOPBACK_EXTERNAL:
      break;
  }

  return ARM_DRIVER_OK;
}

/**
 * @brief  Reads detailed technical processing channel layout capabilities.
 * @param  obj_idx: Numerical channel target identifier index queried.
 * @return ARM_CAN_OBJ_CAPABILITIES: Capabilities bit arrays structure.
 */
static ARM_CAN_OBJ_CAPABILITIES ARM_CAN_ObjectGetCapabilities (uint32_t obj_idx) {
  return can_object_capabilities;
}

/**
 * @brief  Configures acceptance parameters inside hardware Filter Bank 0.
 * @param  obj_idx: Target index indicator channel tracking bounds (Strictly limits receiver path 3).
 * @param  operation: Specific structural execution modifier option type filter (@ref ARM_CAN_FILTER_OPERATION).
 * @param  id: Frame absolute message identifier criteria.
 * @param  arg: Filter auxiliary processing mask properties.
 * @retval ARM_DRIVER_OK: Active acceptance matrix parameters adjusted successfully.
 * @retval ARM_DRIVER_ERROR: Power line verification failure.
 * @retval ARM_DRIVER_ERROR_PARAMETER: Parameter query targeting an index away from object channel 3.
 * @retval ARM_DRIVER_ERROR_UNSUPPORTED: Requested unsupported complex masking filters.
 */
static int32_t ARM_CAN_ObjectSetFilter (uint32_t obj_idx, ARM_CAN_FILTER_OPERATION operation, uint32_t id, uint32_t arg) {

  if (can_driver_powered == 0U) { return ARM_DRIVER_ERROR; }

  if (obj_idx != 3U) { return ARM_DRIVER_ERROR_PARAMETER; }

  switch (operation) {
    case ARM_CAN_FILTER_ID_EXACT_ADD:
      /* Enter Filter Initialization mode by toggling FINIT bit */
      CAN1->FMR |= CAN_FMR_FINIT;
      
      /* Deactivate Filter Bank 0 before changing bit mappings */
      CAN1->FA1R &= ~(1U << 0);

      /* Structural Scale Layout: Single 32-bit scale configuration, Identifier List Mode, Assigned to FIFO 0 */
      CAN1->FS1R  |=  (1U << 0);
      CAN1->FM1R  &= ~(1U << 0); 
      CAN1->FFA1R &= ~(1U << 0);

      /* Clean filter mapping slot registries */
      CAN1->sFilterRegister[0].FR1 = 0x00000000;
      CAN1->sFilterRegister[0].FR2 = 0x00000000;

      /* Reactivate completed Filter Bank 0 and release filter initialization lock */
      CAN1->FA1R |= (1U << 0);
      CAN1->FMR &= ~CAN_FMR_FINIT;
      break;
    case ARM_CAN_FILTER_ID_MASKABLE_ADD:
      break;
    case ARM_CAN_FILTER_ID_RANGE_ADD:
      return ARM_DRIVER_ERROR_UNSUPPORTED;
    case ARM_CAN_FILTER_ID_EXACT_REMOVE:
      return ARM_DRIVER_ERROR_UNSUPPORTED;
    case ARM_CAN_FILTER_ID_MASKABLE_REMOVE:
      return ARM_DRIVER_ERROR_UNSUPPORTED;
    case ARM_CAN_FILTER_ID_RANGE_REMOVE:
      return ARM_DRIVER_ERROR_UNSUPPORTED;
  }

  return ARM_DRIVER_OK;
}

/**
 * @brief  Configures channel messaging profiles.
 * @param  obj_idx: Technical query target tracking channel index indicator.
 * @param  obj_cfg: Operational structural design context format profile selection (@ref ARM_CAN_OBJ_CONFIG).
 * @retval ARM_DRIVER_OK: Properties stored.
 */
static int32_t ARM_CAN_ObjectConfigure (uint32_t obj_idx, ARM_CAN_OBJ_CONFIG obj_cfg) {
  if (can_driver_powered == 0U) { return ARM_DRIVER_ERROR; }
  return ARM_DRIVER_OK;
}

/**
 * @brief  Injects payload frame sequences into target active Tx Mailbox to ignite wire signaling.
 * @param  obj_idx: Targeted Mailbox window index line indicator (Constrained from 0 to 2).
 * @param  msg_info: Struct holding properties like Frame Identifier and ID type flags (Standard/Extended).
 * @param  data: Pointer pointing to raw source byte stream containing message payload elements.
 * @param  size: Total absolute byte lengths containing within stream (Bound to maximum 8 bytes standard limits).
 * @return int32_t: Returns absolute frame length counts committed onto registries, or error codes.
 */
static int32_t ARM_CAN_MessageSend (uint32_t obj_idx, ARM_CAN_MSG_INFO *msg_info, const uint8_t *data, uint8_t size) {

  if (can_driver_powered == 0U) { return ARM_DRIVER_ERROR; }

  if (obj_idx > 2U) { return ARM_DRIVER_ERROR_PARAMETER; }
  
  /* Verify chosen Mailbox window state indicates Transmit Mailbox Empty (TME) */
  if ((CAN1->TSR & (CAN_TSR_TME0 << obj_idx)) == 0) {
      return ARM_DRIVER_ERROR_BUSY;
  }
  
  CAN1->sTxMailBox[obj_idx].TIR = 0;
  
  /* Apply structural alignment bit offsets depending on dynamic Identifier Extended (IDE) mode flags */
  if (msg_info->id & ARM_CAN_ID_IDE_Msk) {
      CAN1->sTxMailBox[obj_idx].TIR = ((msg_info->id & 0x1FFFFFFF) << 3) | CAN_TI0R_IDE;
  } else {
      CAN1->sTxMailBox[obj_idx].TIR = ((msg_info->id & 0x7FF) << 21);
  }

  /* Pack Data Length Code (DLC) specifications */
  CAN1->sTxMailBox[obj_idx].TDTR = (size & 0x0F);
  
  /* Allocate low byte variables data arrays across registers (Bytes 0 to 3) */
  CAN1->sTxMailBox[obj_idx].TDLR = ((uint32_t)data[3] << 24) |
                                   ((uint32_t)data[2] << 16) |
                                   ((uint32_t)data[1] << 8)  |
                                   ((uint32_t)data[0]);

  /* Allocate high byte variables data arrays across registers (Bytes 4 to 7) */
  CAN1->sTxMailBox[obj_idx].TDHR = ((uint32_t)data[7] << 24) |
                                   ((uint32_t)data[6] << 16) |
                                   ((uint32_t)data[5] << 8)  |
                                   ((uint32_t)data[4]);

  /* Commit Transmission Request (TXRQ) bit to launch packet into bus tree lines */
  CAN1->sTxMailBox[obj_idx].TIR |= CAN_TI0R_TXRQ;

  return ((int32_t)size);
}

/**
 * @brief  Pulls fresh messaging frames out from hardware Receive FIFO 0 registries.
 * @param  obj_idx: Technical evaluation channel index query (Constrained to Object Index 3).
 * @param  msg_info: Struct reference to fill with parsed Identifier elements.
 * @param  data: Target storage block allocated to copy message data bytes into.
 * @param  size: Absolute memory bounds allocating byte limits inside destination block.
 * @return int32_t: Returns absolute count lengths extracted cleanly out from registers, or error flags.
 */
static int32_t ARM_CAN_MessageRead (uint32_t obj_idx, ARM_CAN_MSG_INFO *msg_info, uint8_t *data, uint8_t size) {

  if (can_driver_powered == 0U) { return ARM_DRIVER_ERROR;  }

  if (obj_idx != 3U) { return ARM_DRIVER_ERROR_PARAMETER; }

  /* Check Filter Message Pending (FMP) register to ensure packets are waiting inside FIFO 0 */
  if ((CAN1->RF0R & CAN_RF0R_FMP0) == 0) {
      return 0;
  }

  /* Parse out identifier data blocks depending on physical IDE flag settings */
  if (CAN1->sFIFOMailBox[0].RIR & CAN_RI0R_IDE) {
      msg_info->id = (0x1FFFFFFF & (CAN1->sFIFOMailBox[0].RIR >> 3)) | ARM_CAN_ID_IDE_Msk;
  } else {
      msg_info->id = (0x000007FF & (CAN1->sFIFOMailBox[0].RIR >> 21));
  }

  /* Extract framing Data Length Code metrics */
  uint8_t dlc = CAN1->sFIFOMailBox[0].RDTR & CAN_RDT0R_DLC;
  uint8_t read_size = (dlc < size) ? dlc : size;

  uint32_t tdlr = CAN1->sFIFOMailBox[0].RDLR;
  uint32_t tdhr = CAN1->sFIFOMailBox[0].RDHR;

  /* Fetch exact byte streams sequentially from segmented data registers */
  if (read_size > 0) data[0] = (tdlr >> 0)  & 0xFF;
  if (read_size > 1) data[1] = (tdlr >> 8)  & 0xFF;
  if (read_size > 2) data[2] = (tdlr >> 16) & 0xFF;
  if (read_size > 3) data[3] = (tdlr >> 24) & 0xFF;
  if (read_size > 4) data[4] = (tdhr >> 0)  & 0xFF;
  if (read_size > 5) data[5] = (tdhr >> 8)  & 0xFF;
  if (read_size > 6) data[6] = (tdhr >> 16) & 0xFF;
  if (read_size > 7) data[7] = (tdhr >> 24) & 0xFF;

  /* Release FIFO 0 output mailbox by toggling Release FIFO Output Mailbox (RFOM) bit */
  CAN1->RF0R |= CAN_RF0R_RFOM0;

  return ((int32_t)read_size);
}

/**
 * @brief  Controls advanced pipeline execution or aborts messaging requests.
 * @retval ARM_DRIVER_OK: Abort flags parsed.
 * @retval ARM_DRIVER_ERROR_UNSUPPORTED: Unhandled execution control blocks.
 */
static int32_t ARM_CAN_Control (uint32_t control, uint32_t arg) {
  if (can_driver_powered == 0U) { return ARM_DRIVER_ERROR; }

  switch (control & ARM_CAN_CONTROL_Msk) {
    case ARM_CAN_ABORT_MESSAGE_SEND:
      break;
    case ARM_CAN_SET_FD_MODE:
      break;
    case ARM_CAN_SET_TRANSCEIVER_DELAY:
      break;
    default:
      return ARM_DRIVER_ERROR_UNSUPPORTED;
  }

  return ARM_DRIVER_OK;
}

/**
 * @brief  Reads core network status indicators, error logging counters, and bus line diagnostics.
 * @return ARM_CAN_STATUS: Status flags indicators mapping layout.
 */
static ARM_CAN_STATUS ARM_CAN_GetStatus (void) {
  ARM_CAN_STATUS ret = {0};
  return ret;
}

/**
 * @brief  High-Priority Interrupt Service Routine handling CAN Transmission Completion events.
 * @details Monitors Request Completed (RQCP) bits across all 3 transmission mailboxes.
 * Clears flags atomically and fires operational completion callback tokens upwards.
 * @return None
 */
void USB_HP_CAN1_TX_IRQHandler(void) {
    uint32_t tsr = CAN1->TSR;

    /* Check Transmission Request Completed for Mailbox 0 */
    if (tsr & CAN_TSR_RQCP0) {
        CAN1->TSR = CAN_TSR_RQCP0; /* Clear flag by writing 1 */
        if (CAN_SignalObjectEvent) {
          CAN_SignalObjectEvent(0, ARM_CAN_EVENT_SEND_COMPLETE);
        }
    }

    /* Check Transmission Request Completed for Mailbox 1 */
    if (tsr & CAN_TSR_RQCP1) {
        CAN1->TSR = CAN_TSR_RQCP1;
        if (CAN_SignalObjectEvent) {
            CAN_SignalObjectEvent(1, ARM_CAN_EVENT_SEND_COMPLETE);
        }
    }

    /* Check Transmission Request Completed for Mailbox 2 */
    if (tsr & CAN_TSR_RQCP2) {
        CAN1->TSR = CAN_TSR_RQCP2;
        if (CAN_SignalObjectEvent) {
            CAN_SignalObjectEvent(2, ARM_CAN_EVENT_SEND_COMPLETE);
        }
    }
}

/**
 * @brief  Low-Priority Interrupt Service Routine handling CAN Reception events for FIFO 0.
 * @details Detects incoming frames stored inside FIFO 0, triggering the application-layer 
 * receive event dispatch mechanism instantly.
 * @return None
 */
void USB_LP_CAN1_RX0_IRQHandler(void) {
    if ((CAN1->RF0R & CAN_RF0R_FMP0) != 0) {
        if (CAN_SignalObjectEvent) {
            CAN_SignalObjectEvent(3, ARM_CAN_EVENT_RECEIVE); /* Notify object index 3 data arrival */
        }
    }
}

/**
 * @brief Global structure instance for binding CAN Driver capabilities.
 */
ARM_DRIVER_CAN Driver_CAN0 = {
  ARM_CAN_GetVersion,
  ARM_CAN_GetCapabilities,
  ARM_CAN_Initialize,
  ARM_CAN_Uninitialize,
  ARM_CAN_PowerControl,
  ARM_CAN_GetClock,
  ARM_CAN_SetBitrate,
  ARM_CAN_SetMode,
  ARM_CAN_ObjectGetCapabilities,
  ARM_CAN_ObjectSetFilter,
  ARM_CAN_ObjectConfigure,
  ARM_CAN_MessageSend,
  ARM_CAN_MessageRead,
  ARM_CAN_Control,
  ARM_CAN_GetStatus
};
