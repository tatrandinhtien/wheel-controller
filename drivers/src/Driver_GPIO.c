/********************************************************************
* @author: dt
* @email : tien.ta.eswe@gmail.com
* @date  : 25/03/2026
********************************************************************/

#include "Driver_GPIO.h"

#define GPIO_MAX_PINS           37U
#define PIN_IS_AVAILABLE(n)     ((n) < GPIO_MAX_PINS)


/**
 * @brief Set up GPIO interface
 *
 */
static int32_t GPIO_Setup (ARM_GPIO_Pin_t pin, ARM_GPIO_SignalEvent_t cb_event)
{
  int32_t result = ARM_DRIVER_OK;

  if (PIN_IS_AVAILABLE(pin))
  {

  }
  else
  {
    result = ARM_GPIO_ERROR_PIN;
  }

  return result;
}

/**
 * @brief Set GPIO direction
 *
 */
static int32_t GPIO_SetDirection (ARM_GPIO_Pin_t pin, ARM_GPIO_DIRECTION direction)
 {
  int32_t result = ARM_DRIVER_OK;

  if (PIN_IS_AVAILABLE(pin))
  {
    switch (direction)
    {
      case ARM_GPIO_INPUT:
        break;
      case ARM_GPIO_OUTPUT:
        break;
      default:
        result = ARM_DRIVER_ERROR_PARAMETER;
        break;
    }
  }
  else
  {
    result = ARM_GPIO_ERROR_PIN;
  }

  return result;
}

/**
 * @brief Set GPIO output mode
 *
 */
static int32_t GPIO_SetOutputMode (ARM_GPIO_Pin_t pin, ARM_GPIO_OUTPUT_MODE mode)
{
  int32_t result = ARM_DRIVER_OK;

  if (PIN_IS_AVAILABLE(pin))
  {
    switch (mode)
    {
      case ARM_GPIO_PUSH_PULL:
        break;
      case ARM_GPIO_OPEN_DRAIN:
        break;
      default:
        result = ARM_DRIVER_ERROR_PARAMETER;
        break;
    }
  }
  else
  {
    result = ARM_GPIO_ERROR_PIN;
  }

  return result;
}

/**
 * @brief Set GPIO pull resistor
 *
 */
static int32_t GPIO_SetPullResistor (ARM_GPIO_Pin_t pin, ARM_GPIO_PULL_RESISTOR resistor)
{
  int32_t result = ARM_DRIVER_OK;

  if (PIN_IS_AVAILABLE(pin))
  {
    switch (resistor)
    {
      case ARM_GPIO_PULL_NONE:
        break;
      case ARM_GPIO_PULL_UP:
        break;
      case ARM_GPIO_PULL_DOWN:
        break;
      default:
        result = ARM_DRIVER_ERROR_PARAMETER;
        break;
    }
  }
  else
  {
    result = ARM_GPIO_ERROR_PIN;
  }

  return result;
}

/**
 * @brief Set GPIO event trigger
 *
 */
static int32_t GPIO_SetEventTrigger (ARM_GPIO_Pin_t pin, ARM_GPIO_EVENT_TRIGGER trigger)
{
  int32_t result = ARM_DRIVER_OK;

  if (PIN_IS_AVAILABLE(pin))
  {
    switch (trigger)
    {
      case ARM_GPIO_TRIGGER_NONE:
        break;
      case ARM_GPIO_TRIGGER_RISING_EDGE:
        break;
      case ARM_GPIO_TRIGGER_FALLING_EDGE:
        break;
      case ARM_GPIO_TRIGGER_EITHER_EDGE:
        break;
      default:
        result = ARM_DRIVER_ERROR_PARAMETER;
        break;
    }
  }
  else
  {
    result = ARM_GPIO_ERROR_PIN;
  }

  return result;
}

/**
 * @brief Set GPIO output
 *
 */
static void GPIO_SetOutput (ARM_GPIO_Pin_t pin, uint32_t val)
{

  if (PIN_IS_AVAILABLE(pin))
  {
  }
}

/**
 * @brief Get GPIO input
 *
 */
static uint32_t GPIO_GetInput (ARM_GPIO_Pin_t pin)
{
  uint32_t val = 0U;

  if (PIN_IS_AVAILABLE(pin))
  {
  }
  return val;
}


/**
 * @brief GPIO Driver structure
 *
 */
ARM_DRIVER_GPIO Driver_GPIO0 = {
  GPIO_Setup,
  GPIO_SetDirection,
  GPIO_SetOutputMode,
  GPIO_SetPullResistor,
  GPIO_SetEventTrigger,
  GPIO_SetOutput,
  GPIO_GetInput
};
