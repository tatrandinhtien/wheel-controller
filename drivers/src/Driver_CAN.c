/*
 * Copyright (c) 2015-2020 Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Licensed under the Apache License, Version 2.0 (the License); you may
 * not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an AS IS BASIS, WITHOUT
 * WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "stm32f103xb.h"

#include "Driver_CAN.h"
#include "Driver_RCC.h"
#include "Driver_GPIO.h"

extern ARM_DRIVER_GPIO Driver_GPIO0;

#define ARM_CAN_DRV_VERSION ARM_DRIVER_VERSION_MAJOR_MINOR(1,0) // CAN driver version

// Driver Version
static const ARM_DRIVER_VERSION can_driver_version = { ARM_CAN_API_VERSION, ARM_CAN_DRV_VERSION };

// Driver Capabilities
static const ARM_CAN_CAPABILITIES can_driver_capabilities = {
  32U,  // Number of CAN Objects available (mailbox + FIFO)
  0U,   // Does not support reentrant calls 
  0U,   // Does not support CAN FD
  0U,   // Does not support restricted operation mode
  0U,   // Does not support bus monitoring mode
  1U,   // Supports internal loopback mode
  1U,   // Supports external loopback mode
  0U    // Reserved 
};

// Object Capabilities
static const ARM_CAN_OBJ_CAPABILITIES can_object_capabilities = {
  1U,   // Object supports transmission
  1U,   // Object supports reception
  0U,   // RTR rx automatic tx data not supported
  0U,   // RTR tx automatic rx data not supported
  0U,   // Multiple filters not supported in single object simply
  0U,   // Exact filtering not specified simply
  0U,   // Range filtering not specified simply
  0U,   // Mask filtering not specified simply
  0U,   // Buffered messages not supported
  0U    // Reserved
};

static uint8_t                     can_driver_powered     = 0U;
static uint8_t                     can_driver_initialized = 0U;
static ARM_CAN_SignalUnitEvent_t   CAN_SignalUnitEvent    = NULL;
static ARM_CAN_SignalObjectEvent_t CAN_SignalObjectEvent  = NULL;

//
//   Functions
//

static ARM_DRIVER_VERSION ARM_CAN_GetVersion (void) {
  return can_driver_version;
}

static ARM_CAN_CAPABILITIES ARM_CAN_GetCapabilities (void) {
  return can_driver_capabilities;
}

static int32_t ARM_CAN_Initialize (ARM_CAN_SignalUnitEvent_t   cb_unit_event,
                                   ARM_CAN_SignalObjectEvent_t cb_object_event) {

  if (can_driver_initialized != 0U) { return ARM_DRIVER_OK; }

  CAN_SignalUnitEvent   = cb_unit_event;
  CAN_SignalObjectEvent = cb_object_event;

  // Enable AFIO clock for CAN remapping if necessary
  RCC->APB2ENR |= RCC_APB2ENR_AFIOEN;

  // CAN_RX (PA11) - Input Floating
  Driver_GPIO0.Setup(11, NULL);
  Driver_GPIO0.SetDirection(11, ARM_GPIO_INPUT);
  Driver_GPIO0.SetPullResistor(11, ARM_GPIO_PULL_NONE);

  // CAN_TX (PA12) - Alternate Function Push-Pull
  Driver_GPIO0.Setup(12, NULL);
  Driver_GPIO0.SetDirection(12, ARM_GPIO_AF_OUTPUT);
  Driver_GPIO0.SetOutputMode(12, ARM_AFIO_PUSH_PULL);

  can_driver_initialized = 1U;

  return ARM_DRIVER_OK;
}

static int32_t ARM_CAN_Uninitialize (void) {
  can_driver_initialized = 0U;
  return ARM_DRIVER_OK;
}

static int32_t ARM_CAN_PowerControl (ARM_POWER_STATE state) {
  switch (state) {
    case ARM_POWER_OFF:
      can_driver_powered = 0U;
      CAN1->MCR |= CAN_MCR_SLEEP;    // Enable Sleep mode
      RCC->APB1ENR &= ~RCC_APB1ENR_CAN1EN; // Disable clock
      NVIC_DisableIRQ(USB_LP_CAN1_RX0_IRQn);
      NVIC_DisableIRQ(USB_HP_CAN1_TX_IRQn);
      break;

    case ARM_POWER_FULL:
      if (can_driver_initialized == 0U) { return ARM_DRIVER_ERROR; }
      if (can_driver_powered     != 0U) { return ARM_DRIVER_OK;    }

      RCC->APB1ENR |= RCC_APB1ENR_CAN1EN; // Enable CAN clock
      CAN1->MCR &= ~CAN_MCR_SLEEP;   // Wake up CAN

      // Initialization request
      CAN1->MCR |= CAN_MCR_INRQ;
      while ((CAN1->MSR & CAN_MSR_INAK) == 0U);

      // Automatic Bus-off Management
      CAN1->MCR |= CAN_MCR_ABOM;

      // Enable Interrupts on NVIC
      NVIC_EnableIRQ(USB_LP_CAN1_RX0_IRQn);
      NVIC_EnableIRQ(USB_HP_CAN1_TX_IRQn);

      can_driver_powered = 1U;
      break;

    case ARM_POWER_LOW:
      return ARM_DRIVER_ERROR_UNSUPPORTED;
  }

  return ARM_DRIVER_OK;
}

static uint32_t ARM_CAN_GetClock (void) {
  return 36000000; // Return PCLK1 (typically 36MHz for 72MHz system clock)
}

static int32_t ARM_CAN_SetBitrate (ARM_CAN_BITRATE_SELECT select, uint32_t bitrate, uint32_t bit_segments) {
  if (can_driver_powered == 0U) { return ARM_DRIVER_ERROR; }
  if (select != ARM_CAN_BITRATE_NOMINAL) { return ARM_DRIVER_ERROR_UNSUPPORTED; }

  uint32_t pclk = ARM_CAN_GetClock();
  uint32_t brp = 0;
  uint32_t ts1 = 0;
  uint32_t ts2 = 0;
  uint32_t sjw = 1;

  // Search for adequate pre-scaler and time quanta (tq)
  // Target a sample point of ~75-80%
  for (uint32_t tq = 8; tq <= 25; tq++) {
      if (pclk % (bitrate * tq) == 0) {
          uint32_t temp_brp = pclk / (bitrate * tq);
          uint32_t temp_ts1 = (tq * 8) / 10 - 1; // ~80% sample point
          
          if (temp_ts1 > 16) temp_ts1 = 16;
          uint32_t temp_ts2 = tq - 1 - temp_ts1;
          
          if (temp_ts2 < 1) { 
              temp_ts2 = 1; 
              temp_ts1 = tq - 1 - temp_ts2; 
          }
          
          if (temp_ts1 >= 1 && temp_ts1 <= 16 && temp_ts2 >= 1 && temp_ts2 <= 8 && temp_brp <= 1024) {
              brp = temp_brp;
              ts1 = temp_ts1;
              ts2 = temp_ts2;
              break;
          }
      }
  }

  if (brp == 0 || brp > 1024) return ARM_DRIVER_ERROR_UNSUPPORTED;

  CAN1->MCR |= CAN_MCR_INRQ;
  while ((CAN1->MSR & CAN_MSR_INAK) == 0U);

  CAN1->BTR = ((sjw - 1) << 24) |
              ((ts2 - 1) << 20) |
              ((ts1 - 1) << 16) |
              ((brp - 1) << 0);

  return ARM_DRIVER_OK;
}

static int32_t ARM_CAN_SetMode (ARM_CAN_MODE mode) {
  if (can_driver_powered == 0U) { return ARM_DRIVER_ERROR; }

  // Must enter Initialization Mode before modifying BTR
  CAN1->MCR |= CAN_MCR_INRQ;
  while ((CAN1->MSR & CAN_MSR_INAK) == 0U);

  // Always clear Loopback / Silent bits first
  CAN1->BTR &= ~(CAN_BTR_LBKM | CAN_BTR_SILM);

  switch (mode) {
    case ARM_CAN_MODE_INITIALIZATION:
      // Already in Init mode
      return ARM_DRIVER_OK;

    case ARM_CAN_MODE_NORMAL:
      CAN1->MCR &= ~CAN_MCR_INRQ;
      while ((CAN1->MSR & CAN_MSR_INAK) != 0U);
      return ARM_DRIVER_OK;

    case ARM_CAN_MODE_RESTRICTED: // Silent Mode
      CAN1->BTR |= CAN_BTR_SILM;
      CAN1->MCR &= ~CAN_MCR_INRQ;
      while ((CAN1->MSR & CAN_MSR_INAK) != 0U);
      return ARM_DRIVER_OK;

    case ARM_CAN_MODE_LOOPBACK_INTERNAL:
      CAN1->BTR |= CAN_BTR_LBKM;
      CAN1->MCR &= ~CAN_MCR_INRQ;
      while ((CAN1->MSR & CAN_MSR_INAK) != 0U);
      return ARM_DRIVER_OK;

    default:
      return ARM_DRIVER_ERROR_UNSUPPORTED;
  }
}

static ARM_CAN_OBJ_CAPABILITIES ARM_CAN_ObjectGetCapabilities (uint32_t obj_idx) {
  return can_object_capabilities;
}

static int32_t ARM_CAN_ObjectSetFilter (uint32_t obj_idx, ARM_CAN_FILTER_OPERATION operation, uint32_t id, uint32_t arg) {
  if (can_driver_powered == 0U) { return ARM_DRIVER_ERROR; }

  uint32_t filter_idx = obj_idx % 14; // STM32F103 has 14 filter banks

  CAN1->FMR |= CAN_FMR_FINIT;

  if (operation == ARM_CAN_FILTER_ID_EXACT_REMOVE || 
      operation == ARM_CAN_FILTER_ID_MASKABLE_REMOVE || 
      operation == ARM_CAN_FILTER_ID_RANGE_REMOVE) {
      CAN1->FA1R &= ~(1U << filter_idx); // Deactivate filter
  } else {
      CAN1->FS1R |= (1U << filter_idx);  // 32-bit scale
      CAN1->FM1R &= ~(1U << filter_idx); // Mask mode
      
      // Standard ID is shifted 21 bits. Extended is shifted 3 bits.
      if (id & ARM_CAN_ID_IDE_Msk) {
          CAN1->sFilterRegister[filter_idx].FR1 = ((id & ~ARM_CAN_ID_IDE_Msk) << 3) | 4;
          CAN1->sFilterRegister[filter_idx].FR2 = ((arg & ~ARM_CAN_ID_IDE_Msk) << 3) | 4;
      } else {
          CAN1->sFilterRegister[filter_idx].FR1 = (id << 21);
          CAN1->sFilterRegister[filter_idx].FR2 = (arg << 21);
      }

      CAN1->FFA1R &= ~(1U << filter_idx); // Assign to FIFO 0
      CAN1->FA1R |= (1U << filter_idx);   // Activate filter
  }

  CAN1->FMR &= ~CAN_FMR_FINIT;

  return ARM_DRIVER_OK;
}

static int32_t ARM_CAN_ObjectConfigure (uint32_t obj_idx, ARM_CAN_OBJ_CONFIG obj_cfg) {
  if (can_driver_powered == 0U) { return ARM_DRIVER_ERROR; }

  switch (obj_cfg) {
    case ARM_CAN_OBJ_TX:
      CAN1->IER |= CAN_IER_TMEIE; // Enable TX interrupt
      break;
    case ARM_CAN_OBJ_RX:
      CAN1->IER |= CAN_IER_FMPIE0; // Enable FIFO0 RX interrupt
      break;
    default:
      break;
  }
  return ARM_DRIVER_OK;
}

static int32_t ARM_CAN_MessageSend (uint32_t obj_idx, ARM_CAN_MSG_INFO *msg_info, const uint8_t *data, uint8_t size) {
  if (can_driver_powered == 0U) { return ARM_DRIVER_ERROR; }

  uint8_t mbx = 0;
  if (CAN1->TSR & CAN_TSR_TME0) mbx = 0;
  else if (CAN1->TSR & CAN_TSR_TME1) mbx = 1;
  else if (CAN1->TSR & CAN_TSR_TME2) mbx = 2;
  else return ARM_DRIVER_ERROR_BUSY;

  uint8_t dlc = (size > 8) ? 8 : (size & 0x0F);

  CAN1->sTxMailBox[mbx].TIR = 0;
  CAN1->sTxMailBox[mbx].TDTR = dlc;

  if (msg_info->id & ARM_CAN_ID_IDE_Msk) {
    CAN1->sTxMailBox[mbx].TIR |= ((msg_info->id & ~ARM_CAN_ID_IDE_Msk) << 3) | CAN_TI0R_IDE;
  } else {
    CAN1->sTxMailBox[mbx].TIR |= (msg_info->id << 21);
  }

  if (msg_info->rtr) {
    CAN1->sTxMailBox[mbx].TIR |= CAN_TI0R_RTR;
  } else {
    uint32_t data_l = 0;
    uint32_t data_h = 0;
    for (int i = 0; i < 4; i++) {
        if (i < dlc) data_l |= ((uint32_t)data[i] << (i * 8));
    }
    for (int i = 4; i < 8; i++) {
        if (i < dlc) data_h |= ((uint32_t)data[i] << ((i - 4) * 8));
    }
    CAN1->sTxMailBox[mbx].TDLR = data_l;
    CAN1->sTxMailBox[mbx].TDHR = data_h;
  }

  CAN1->sTxMailBox[mbx].TIR |= CAN_TI0R_TXRQ;

  return ((int32_t)size);
}

static int32_t ARM_CAN_MessageRead (uint32_t obj_idx, ARM_CAN_MSG_INFO *msg_info, uint8_t *data, uint8_t size) {
  if (can_driver_powered == 0U) { return ARM_DRIVER_ERROR;  }
  if ((CAN1->RF0R & CAN_RF0R_FMP0) == 0U) { return 0U; } // No message

  if (CAN1->sFIFOMailBox[0].RIR & CAN_RI0R_IDE) {
    msg_info->id = (CAN1->sFIFOMailBox[0].RIR >> 3) | ARM_CAN_ID_IDE_Msk;
  } else {
    msg_info->id = CAN1->sFIFOMailBox[0].RIR >> 21;
  }

  msg_info->rtr = (CAN1->sFIFOMailBox[0].RIR & CAN_RI0R_RTR) ? 1 : 0;
  msg_info->dlc = CAN1->sFIFOMailBox[0].RDTR & CAN_RDT0R_DLC;

  uint8_t count = msg_info->dlc;
  if (count > size) { count = size; }
  if (count > 8) { count = 8; }

  if (!msg_info->rtr) {
    if (count > 0) data[0] = (uint8_t)(CAN1->sFIFOMailBox[0].RDLR);
    if (count > 1) data[1] = (uint8_t)(CAN1->sFIFOMailBox[0].RDLR >> 8);
    if (count > 2) data[2] = (uint8_t)(CAN1->sFIFOMailBox[0].RDLR >> 16);
    if (count > 3) data[3] = (uint8_t)(CAN1->sFIFOMailBox[0].RDLR >> 24);
    if (count > 4) data[4] = (uint8_t)(CAN1->sFIFOMailBox[0].RDHR);
    if (count > 5) data[5] = (uint8_t)(CAN1->sFIFOMailBox[0].RDHR >> 8);
    if (count > 6) data[6] = (uint8_t)(CAN1->sFIFOMailBox[0].RDHR >> 16);
    if (count > 7) data[7] = (uint8_t)(CAN1->sFIFOMailBox[0].RDHR >> 24);
  }

  CAN1->RF0R |= CAN_RF0R_RFOM0; // Release FIFO0
  CAN1->IER |= CAN_IER_FMPIE0;  // Re-enable RX interrupt for FIFO0

  return ((int32_t)count);
}

static int32_t ARM_CAN_Control (uint32_t control, uint32_t arg) {
  if (can_driver_powered == 0U) { return ARM_DRIVER_ERROR; }
  return ARM_DRIVER_OK;
}

static ARM_CAN_STATUS ARM_CAN_GetStatus (void) {
  ARM_CAN_STATUS stat = {0};
  
  if (CAN1->ESR & CAN_ESR_BOFF) {
      stat.unit_state = ARM_CAN_UNIT_STATE_BUS_OFF;
  } else if (CAN1->ESR & CAN_ESR_EPVF) {
      stat.unit_state = ARM_CAN_UNIT_STATE_PASSIVE;
  } else if (CAN1->ESR & CAN_ESR_EWGF) {
      stat.unit_state = ARM_CAN_UNIT_STATE_ACTIVE;
  } else {
      stat.unit_state = ARM_CAN_UNIT_STATE_ACTIVE;
  }

  stat.last_error_code = (CAN1->ESR & CAN_ESR_LEC) >> CAN_ESR_LEC_Pos;
  stat.rx_error_count = (CAN1->ESR & CAN_ESR_REC) >> CAN_ESR_REC_Pos;
  stat.tx_error_count = (CAN1->ESR & CAN_ESR_TEC) >> CAN_ESR_TEC_Pos;

  return stat;
}

// IRQ handlers
void USB_LP_CAN1_RX0_IRQHandler(void) {
  if (CAN1->RF0R & CAN_RF0R_FMP0) {
    if (CAN_SignalObjectEvent) {
      CAN_SignalObjectEvent(0, ARM_CAN_EVENT_RECEIVE);
    }
    // Disable RX interrupt to prevent interrupt storm until read
    CAN1->IER &= ~CAN_IER_FMPIE0;
  }
}

void USB_HP_CAN1_TX_IRQHandler(void) {
  uint32_t tsr = CAN1->TSR;
  if (tsr & CAN_TSR_RQCP0) CAN1->TSR |= CAN_TSR_RQCP0;
  if (tsr & CAN_TSR_RQCP1) CAN1->TSR |= CAN_TSR_RQCP1;
  if (tsr & CAN_TSR_RQCP2) CAN1->TSR |= CAN_TSR_RQCP2;
  
  if (CAN_SignalObjectEvent) {
    CAN_SignalObjectEvent(0, ARM_CAN_EVENT_SEND_COMPLETE);
  }
}

// CAN driver functions structure

extern \
ARM_DRIVER_CAN Driver_CAN0;
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


