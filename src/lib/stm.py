## @file
## @package stm
#  @brief STM32F0 Nucleo board Access Module
#
# Provides generic access to the V
#
#
# <b>USAGE</b>
#
# \code
# import stm
# stm.setLED(2) 	#Power on LED2
# stm.resetLED(3) 	#Power off LED3
# stm.delay(500) 	#Half second pause
# \endcode


"""__NATIVE__
#include <stm32f0xx_hal.h>

/*
 * Common methods
 */


"""


# Power off a led present on board
#
# lednum could be 2 or 3 that are the names of the LED loaded
# on STM32F0 Nucleo board
#
def resetLED(lednum):
    """__NATIVE__
    PmReturn_t retval = PM_RET_OK;
    GPIO_TypeDef* GPIOx;
    uint16_t GPIO_Pin;
    pPmObj_t pLedNum;

    NATIVE_SET_TOS(PM_NONE);

    if(NATIVE_GET_NUM_ARGS() != 1)
    {
      PM_RAISE(retval, PM_RET_EX_TYPE);
      return retval;
    }

    pLedNum = NATIVE_GET_LOCAL(0);
    /* Raise a TypeError if 1nd arg is not an int */
    if (OBJ_GET_TYPE(pLedNum) != OBJ_TYPE_INT)
    {
      PM_RAISE(retval, PM_RET_EX_TYPE);
      return retval;
    }

    if(((pPmInt_t)pLedNum)->val == 2)
    {
      GPIOx = GPIOA;
      GPIO_Pin = LED2_PIN;
    }
    else if (((pPmInt_t)pLedNum)->val == 3)
    {
      GPIOx = GPIOE;
      GPIO_Pin = GPIO_PIN_7;
    }
    else
    {
      PM_RAISE(retval, PM_RET_EX_TYPE);
      return retval;
    }

    HAL_GPIO_WritePin(GPIOx, GPIO_Pin, 0);

    return retval;
    """
    pass


# Power on a led present on board
#
# lednum could be 2 or 3 that are the names of the LED loaded
# on STM32F0 Nucleo board
#
def setLED(lednum):
    """__NATIVE__
    PmReturn_t retval = PM_RET_OK;
    GPIO_TypeDef* GPIOx;
    uint16_t GPIO_Pin;
    pPmObj_t pLedNum;

    NATIVE_SET_TOS(PM_NONE);

    if(NATIVE_GET_NUM_ARGS() != 1)
    {
      PM_RAISE(retval, PM_RET_EX_TYPE);
      return retval;
    }

    pLedNum = NATIVE_GET_LOCAL(0);
    /* Raise a TypeError if 1nd arg is not an int */
    if (OBJ_GET_TYPE(pLedNum) != OBJ_TYPE_INT)
    {
      PM_RAISE(retval, PM_RET_EX_TYPE);
      return retval;
    }

    if(((pPmInt_t)pLedNum)->val == 2)
    {
      GPIOx = GPIOA;
      GPIO_Pin = LED2_PIN;
    }
    else if (((pPmInt_t)pLedNum)->val == 3)
    {
      GPIOx = GPIOE;
      GPIO_Pin = GPIO_PIN_7;
    }
    else
    {
      PM_RAISE(retval, PM_RET_EX_TYPE);
      return retval;
    }

    HAL_GPIO_WritePin(GPIOx, GPIO_Pin, 1);

    return retval;
    """
    pass


def delay(ms):
    """__NATIVE__
    PmReturn_t retval = PM_RET_OK;

    if(NATIVE_GET_NUM_ARGS() != 1)
    {
      PM_RAISE(retval, PM_RET_EX_TYPE);
      return retval;
    }

    pPmObj_t pa = NATIVE_GET_LOCAL(0);
    if (OBJ_GET_TYPE(pa) == OBJ_TYPE_INT)
    {
      HAL_Delay((double) ((pPmInt_t)pa)->val);
    }
    else
    {
      PM_RAISE(retval, PM_RET_EX_TYPE);
    }

    NATIVE_SET_TOS(PM_NONE);
    return retval;
    """
    pass



# :mode=c:
