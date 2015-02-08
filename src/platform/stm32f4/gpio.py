# This file is Copyright 2012 JMSI
#
## @file
#  @copybrief gpio
#
## @package gpio
#  @brief STM32 GPIO Access Module
#
# Provides generic access to the stm32 GPIOs

"""__NATIVE__
#include "stdio.h"
#include "plat.h"
#include "stm32f4xx.h"
#include "stm32f4xx_rcc.h"
#include "stm32f4xx_gpio.h"
#include "system_stm32f4xx.h"
#include "stm32f4xx_tim.h"

#define HIGH Bit_SET
#define LOW Bit_RESET

#define _digitalWrite(GPIOx, GPIO_Pin, BitVal) { \
    GPIO_WriteBit(GPIOx, GPIO_Pin, BitVal); \
}

/*
 * Timer init
 * for PWM purpose, for the moment only tow timer are available TIM1 and TIM8.
 * They are dedicated for PWM feature. 
 *
 * TODO : manage other timer
 *
 * TIM1_CH1 => PA8
 * TIM1_CH2 => PA9
 * TIM1_CH3 => PA10
 * TIM1_CH4 => PA11
 *
 * TIM8_CH1 => PC6
 * TIM8_CH2 => PC7
 * TIM8_CH3 => PC8
 * TIM8_CH4 => PC9
 *
*/
PmReturn_t  _init_timer(TIM_TypeDef* TIMx){
  PmReturn_t retval = PM_RET_OK;

  /* compute the prescaler value */
  // Get clock to 1 MHz on STM32F4
  uint16_t prescalerValue = (uint16_t) ((SystemCoreClock / 1000000)) - 1;
  pPmObj_t p_period;

  // check if third parameter is the period
  // for example period can be set to 20000 (20 KHz)
  p_period = NATIVE_GET_LOCAL(2);
  if (OBJ_GET_TYPE(p_period) != OBJ_TYPE_INT)
  {
    PM_RAISE(retval, PM_RET_EX_TYPE);
    return retval;
  }

  // 20 KHz for 1MHz prescaled
  uint32_t period = (1000000 / ((pPmInt_t)p_period)->val ) - 1;

  TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;
  /* Time base configuration */
  TIM_TimeBaseStructure.TIM_Period = period;
  TIM_TimeBaseStructure.TIM_Prescaler = prescalerValue;
  TIM_TimeBaseStructure.TIM_ClockDivision = 0;
  TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;

  /* TIMx clock enable */
  if (TIMx == TIM1){
        RCC_APB2PeriphClockCmd(RCC_APB2Periph_TIM1, ENABLE);
  }  
  else if (TIMx == TIM8)
  {      
        RCC_APB2PeriphClockCmd(RCC_APB2Periph_TIM8, ENABLE);
  } else {
        PM_RAISE(retval, PM_RET_EX_VAL);
        return retval;
  }
  TIM_TimeBaseInit(TIMx, &TIM_TimeBaseStructure);
  /* Enable TIM 1 or 8 Preload register on ARR */
  TIM_ARRPreloadConfig(TIMx, ENABLE);
}

/*
 *  init PWM mode for timer ...
 */
PmReturn_t  _init_pwm(TIM_TypeDef* TIMx, uint8_t channel_num){
  PmReturn_t retval = PM_RET_OK;

  TIM_OCInitTypeDef TIM_OCInitStructure;

  // The difference between PWM1 and PWM2 is whether you are controlling the ON or OFF periods
  // We are going to controle OFF
  // this means, if value grow, led will shine :)
  TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM2;
  TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable;

  // The TIM_Pulse defines the delay value 
  TIM_OCInitStructure.TIM_Pulse = 0;
  TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_High;

  if (TIMx == TIM1){
    if (channel_num==1){
        TIM_OC1Init(TIM1, &TIM_OCInitStructure);
        TIM_OC1PreloadConfig(TIM1, TIM_OCPreload_Enable);
    } else if (channel_num==2){
        TIM_OC2Init(TIM1, &TIM_OCInitStructure);
        TIM_OC2PreloadConfig(TIM1, TIM_OCPreload_Enable);
    } else if (channel_num==3){
        TIM_OC3Init(TIM1, &TIM_OCInitStructure);
        TIM_OC3PreloadConfig(TIM1, TIM_OCPreload_Enable);
    } else if (channel_num==4){
        TIM_OC4Init(TIM1, &TIM_OCInitStructure);
        TIM_OC4PreloadConfig(TIM1, TIM_OCPreload_Enable);
    }
  }  
  else if (TIMx == TIM8)
  {      
    if (channel_num==1){
        TIM_OC1Init(TIM8, &TIM_OCInitStructure);
        TIM_OC1PreloadConfig(TIM8, TIM_OCPreload_Enable);
    } else if (channel_num==2){
        TIM_OC2Init(TIM8, &TIM_OCInitStructure);
        TIM_OC2PreloadConfig(TIM8, TIM_OCPreload_Enable);
    } else if (channel_num==3){
        TIM_OC3Init(TIM8, &TIM_OCInitStructure);
        TIM_OC3PreloadConfig(TIM8, TIM_OCPreload_Enable);
    } else if (channel_num==4){
        TIM_OC4Init(TIM8, &TIM_OCInitStructure);
        TIM_OC4PreloadConfig(TIM8, TIM_OCPreload_Enable);
    }
  } else {
        PM_RAISE(retval, PM_RET_EX_VAL);
        return retval;
  }

  /* TIM 1 or 8 Main Output Enable */
  TIM_CtrlPWMOutputs(TIMx, ENABLE);
  TIM_Cmd(TIMx, ENABLE);

  return retval;
}

/**
*   set the output value for the timer  this is shortcut for :
*       normaly period should be computed like so :
*       pulse = (percent_value * (period - 1)) / 100
*       TIMx->CCRx = value;
*       where TIMx is TIM1 for example
*            and CCRx is CCR1 for channel 1 for example
*
**/
void _set_time_compare(TIM_TypeDef* TIMx, uint8_t channel_num, uint32_t value){
  uint32_t current_period = TIMx->ARR;
  uint32_t pulse = (value*(current_period-1))/100;

  if (TIMx == TIM1){
    if (channel_num==1){
        TIM_SetCompare1(TIM1, pulse);
    } else if (channel_num==2){
        TIM_SetCompare2(TIM1, pulse);
    } else if (channel_num==3){
        TIM_SetCompare3(TIM1, pulse);
    } else if (channel_num==4){
        TIM_SetCompare4(TIM1, pulse);
    }
  } else if (TIMx == TIM8){
    if (channel_num==1){
        TIM_SetCompare1(TIM8, pulse);
    } else if (channel_num==2){
        TIM_SetCompare2(TIM8, pulse);
    } else if (channel_num==3){
        TIM_SetCompare3(TIM8, pulse);
    } else if (channel_num==4){
        TIM_SetCompare4(TIM8, pulse);
    }
  }
}

/*
 * Loads the correct STM32 GPIOs from the first
 * Python argument, and integer pin number (0-15) from second argument.
 * Port name argument is expected to be a single-character string with the port
 * letter ([a-dA-D])
 *
 * port_reg argument is optional.
 */
PmReturn_t  _get_port_register(GPIO_TypeDef **port_reg,
                               uint16_t *pin_mask,
                               uint8_t *pin_num)
{
    pPmObj_t pa;
    pPmObj_t pb;
    PmReturn_t retval = PM_RET_OK;

    pa = NATIVE_GET_LOCAL(0);
    if (OBJ_GET_TYPE(pa) != OBJ_TYPE_STR)
    {
      PM_RAISE(retval, PM_RET_EX_TYPE);
      return retval;
    }

    pb = NATIVE_GET_LOCAL(1);
    if (OBJ_GET_TYPE(pb) != OBJ_TYPE_INT)
    {
      PM_RAISE(retval, PM_RET_EX_TYPE);
      return retval;
    }

    // Only single-character strings for the port number
    if ((((pPmString_t)pa)->length) != 1)
    {
      PM_RAISE(retval, PM_RET_EX_VAL);
      return retval;
    }

    // Find port & direction regs (TODO: Possibly make a PROGMEM lookup table)
    switch(((pPmString_t)pa)->val[0])
    {
      case 'a':
      case 'A':
        if(port_reg) *port_reg = GPIOA;
        break;
      case 'b':
      case 'B':
        if(port_reg) *port_reg = GPIOB;
        break;
      case 'c':
      case 'C':
        if(port_reg) *port_reg = GPIOC;
        break;
      case 'd':
      case 'D':
        if(port_reg) *port_reg = GPIOD;
        break;
      case 'e':
      case 'E':
        if(port_reg) *port_reg = GPIOE;
        break;
      case 'f':
      case 'F':
        if(port_reg) *port_reg = GPIOF;
        break;
      default:
        PM_RAISE(retval, PM_RET_EX_VAL);
        return retval;
    }

    // Check pin is in range
    if(((pPmInt_t)pb)->val < 0 || ((pPmInt_t)pb)->val > 15)
    {
        PM_RAISE(retval, PM_RET_EX_VAL);
        return retval;
    }
    *pin_mask = 1<<((pPmInt_t)pb)->val;
    *pin_num = ((pPmInt_t)pb)->val;

    return retval;
}


PmReturn_t change_port_state(uint8_t state){

    pPmObj_t pa;
    PmReturn_t retval = PM_RET_OK;
    
    if(NATIVE_GET_NUM_ARGS() != 1)
    {
      PM_RAISE(retval, PM_RET_EX_TYPE);
      return retval;
    }
    
    pa = NATIVE_GET_LOCAL(0);
    if (OBJ_GET_TYPE(pa) != OBJ_TYPE_STR)
    {
      PM_RAISE(retval, PM_RET_EX_TYPE);
      return retval;
    }
    
    // Find port & direction regs (TODO: Possibly make a PROGMEM lookup table)
    switch(((pPmString_t)pa)->val[0])
    {
      case 'a':
      case 'A':
        RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, state);
        break;
      case 'b':
      case 'B':
        RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB, state);
        break;
      case 'c':
      case 'C':
        RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOC, state);
        break;
      case 'd':
      case 'D':
        RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOD, state);
        break;
      case 'e':
      case 'E':
        RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOE, state);
        break;
      case 'f':
      case 'F':
        RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOF, state);
        break;
      case 'g':
      case 'G':
        RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOG, state);
        break;
      case 'h':
      case 'H':
        RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOH, state);
        break;
      case 'i':
      case 'I':
        RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOI, state);
        break;
      default:
        PM_RAISE(retval, PM_RET_EX_VAL);
        return retval;
    }
    return retval;
}
"""

INPUT = 0
OUTPUT = 0x01
HIGH = True
LOW = False
    
def enable_port(port):
    """__NATIVE__

    return change_port_state(ENABLE);
    """
    pass
    
def disable_port(port):
    """__NATIVE__
    return change_port_state(DISABLE);
    """
    pass

def _set_pin_pwm(port, pin, period):
    """__NATIVE__
    GPIO_TypeDef *port;
    uint16_t pin_mask;
    uint8_t pin_num;
    TIM_TypeDef* TIMx;
    uint32_t period;
    uint8_t channel;
    uint8_t GPIO_AF_TIM;
    
    PmReturn_t retval = PM_RET_OK;
    
    if(NATIVE_GET_NUM_ARGS() != 3)
    {
      PM_RAISE(retval, PM_RET_EX_TYPE);
      return retval;
    }

    retval = _get_port_register(&port, &pin_mask, &pin_num);
    if(retval != PM_RET_OK)
      return retval;

    /*
    * PA8 => TIM1_CH1 => (TIM1,1)
    * PA9 => TIM1_CH2 => (TIM1,2)
    * PA10 => TIM1_CH3 => (TIM1,3)
    * PA11 => TIM1_CH4 => (TIM1,4)
    *
    * PC6 => TIM8_CH1 => (TIM8,1)
    * PC7 => TIM8_CH2 => (TIM8,2)
    * PC8 => TIM8_CH3 => (TIM8,3)
    * PC9 => TIM8_CH4 => (TIM8,4)
    */

    if (port != GPIOA && port != GPIOC){
      // TODO: find a better exception ...
      PM_RAISE(retval, PM_RET_EX_TYPE);
      return retval;
    }

    // TODO : check if port and pin_num target a correct pin
    // TODO : rewrite method to use init method correctly
    //        usage GPIO_InitStructure

    GPIO_InitTypeDef GPIO_InitStructure;
    GPIO_InitStructure.GPIO_Pin = (uint16_t)pin_num;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
    // GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
    GPIO_Init(port, &GPIO_InitStructure);

    // compute association between port/pin_num and timer !
    // see STM32F4 documentation
    if (port == GPIOA && pin_num == 8){
        TIMx = TIM1;
        channel = 1;
        GPIO_AF_TIM = GPIO_AF_TIM1;
    } else if (port == GPIOA && pin_num == 9){
        TIMx = TIM1;
        channel = 2;
        GPIO_AF_TIM = GPIO_AF_TIM1;
    } else if (port == GPIOA && pin_num == 10){
        TIMx = TIM1;
        channel = 3;
        GPIO_AF_TIM = GPIO_AF_TIM1;
    } else if (port == GPIOA && pin_num == 11){
        TIMx = TIM1;
        channel = 4;
        GPIO_AF_TIM = GPIO_AF_TIM1;
    } else if (port == GPIOC && pin_num == 6){
        TIMx = TIM8;
        channel = 1;
        GPIO_AF_TIM = GPIO_AF_TIM8;
    } else if (port == GPIOC && pin_num == 7){
        TIMx = TIM8;
        channel = 2;
        GPIO_AF_TIM = GPIO_AF_TIM8;
    } else if (port == GPIOC && pin_num == 8){
        TIMx = TIM8;
        channel = 3;
        GPIO_AF_TIM = GPIO_AF_TIM8;
    } else if (port == GPIOC && pin_num == 9){
        TIMx = TIM8;
        channel = 4;
        GPIO_AF_TIM = GPIO_AF_TIM8;
    } else {
      // TODO: find a better exception ...
      PM_RAISE(retval, PM_RET_EX_TYPE);
      return retval;
    }
    // compute timer and channel from port/pin_num

    // init timer
    _init_timer(TIMx);

    if (port==GPIOA){
        RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);
    } else if (port==GPIOC){
        RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOC, ENABLE);
    } else {
      // TODO: find a better exception ...
      PM_RAISE(retval, PM_RET_EX_TYPE);
      return retval;
    }
    // init pwm for the right timer/channel
    _init_pwm(TIMx, channel);

    // associate GPIO pin to timer
    GPIO_PinAFConfig(port, pin_num, GPIO_AF_TIM);
    return retval;
    """
    pass

def set_pin_pwm(pin, period=20000):
    """
    :param pin: tuple for example ('A',8), which shuld be pin.PA8
            (see pin.py and stm32f4 documentation for more details)
    :param period: value in Hz : 20000 for 20 KHz
    """
    _set_pin_pwm(pin[0],pin[1],period)

def _digital_pwm_write(port, pin, percent_pulse):
    """__NATIVE__
    GPIO_TypeDef *port;
    uint16_t pin_mask;
    uint8_t pin_num;
    TIM_TypeDef* TIMx;
    uint8_t channel;
    pPmObj_t pc;
    PmReturn_t retval = PM_RET_OK;

    NATIVE_SET_TOS(PM_NONE);

    if(NATIVE_GET_NUM_ARGS() != 3)
    {
      PM_RAISE(retval, PM_RET_EX_TYPE);
      return retval;
    }

    retval = _get_port_register(&port, &pin_mask, &pin_num);
    if(retval != PM_RET_OK)
      return retval;

    pc = NATIVE_GET_LOCAL(2);

    /* If the arg is not an integer, raise TypeError */
    if (OBJ_GET_TYPE(pc) != OBJ_TYPE_INT)
    {
      PM_RAISE(retval, PM_RET_EX_TYPE);
      return retval;
    }

    if (port != GPIOA && port != GPIOC){
      // TODO: find a better exception ...
      PM_RAISE(retval, PM_RET_EX_TYPE);
      return retval;
    }

    // compute association between port/pin_num and timer !
    // see STM32F4 documentation
    if (port == GPIOA && pin_num == 8){
        TIMx = TIM1;
        channel = 1;
    } else if (port == GPIOA && pin_num == 9){
        TIMx = TIM1;
        channel = 2;
    } else if (port == GPIOA && pin_num == 10){
        TIMx = TIM1;
        channel = 3;
    } else if (port == GPIOA && pin_num == 11){
        TIMx = TIM1;
        channel = 4;
    } else if (port == GPIOC && pin_num == 6){
        TIMx = TIM8;
        channel = 1;
    } else if (port == GPIOC && pin_num == 7){
        TIMx = TIM8;
        channel = 2;
    } else if (port == GPIOC && pin_num == 8){
        TIMx = TIM8;
        channel = 3;
    } else if (port == GPIOC && pin_num == 9){
        TIMx = TIM8;
        channel = 4;
    } else {
      // TODO: find a better exception ...
      PM_RAISE(retval, PM_RET_EX_TYPE);
      return retval;
    }   
    _set_time_compare(TIMx, channel, ((pPmInt_t)pc)->val);
    return retval;
    """
    pass

def digital_pwm_write(pin, percent_pulse):
    """
    pulse should be an integer and a percent
    => between 1 and 100
    """
    if percent_pulse < 0 :
       percent_pulse = 0
    if percent_pulse > 100 :
       percent_pulse = 100
    _digital_pwm_write(pin[0], pin[1], percent_pulse)

# Reads a single pin of a particular STM32 port
#
# Port is specified as a single-character string, A-F.
# Pin is specified as an integer, 0-15
#
# Return value is boolean True/False, can be treated as 1/0
def _digitalRead(port, pin):
    """__NATIVE__
    GPIO_TypeDef *port;
    uint16_t pin_mask;
    uint8_t pin_num;
    
    PmReturn_t retval = PM_RET_OK;
    /*
    if(NATIVE_GET_NUM_ARGS() != 2)
    {
      PM_RAISE(retval, PM_RET_EX_TYPE);
      return retval;
    }

    retval = _get_port_register(&port, &pin_mask, &pin_num);
    if(retval != PM_RET_OK)
      return retval;
    
    pPmObj_t pa = ((port->IDR & pin_mask) != (uint32_t)Bit_RESET) ? PM_TRUE : PM_FALSE;
    NATIVE_SET_TOS(pa); // Push our result object onto the stack*/
    return retval;
    """
    pass

def digitalRead(pin):
    """
    digital read from static pin name
    see pin.py module
    """
    _digitalRead(pin[0],pin[1])
    

def _get_pin_mask(pin_num):
    return 1<<pin_num
    
def configure_pin(port, pin_num):
    pass

# Writes a single pin of a particular STM32 port
#
# Port is specified as a single-character string, A-F.
# Pin is specified as an integer, 0-15
# Value is either boolean True/False or Integer 0 or non-zero.
#
def _digital_write(port, pin, value):
    """__NATIVE__
    GPIO_TypeDef *port;
    uint16_t pin_mask;
    uint8_t pin_num;
    pPmObj_t pc;
    PmReturn_t retval = PM_RET_OK;

    NATIVE_SET_TOS(PM_NONE);

    if(NATIVE_GET_NUM_ARGS() != 3)
    {
      PM_RAISE(retval, PM_RET_EX_TYPE);
      return retval;
    }

    retval = _get_port_register(&port, &pin_mask, &pin_num);
    if(retval != PM_RET_OK)
      return retval;

    pc = NATIVE_GET_LOCAL(2);

    /* If the arg is not an integer, raise TypeError */
    if (OBJ_GET_TYPE(pc) != OBJ_TYPE_INT && OBJ_GET_TYPE(pc) != OBJ_TYPE_BOOL)
    {
      PM_RAISE(retval, PM_RET_EX_TYPE);
      return retval;
    }

    if(((pPmInt_t)pc)->val) {
      //GPIO_SetBits(port, pin_mask);
      port->BSRRL = pin_mask;
    } else {
      //GPIO_ResetBits(port, pin_mask);
      port->BSRRH = pin_mask;
    }
    
    return retval;
    """
    pass
    
def digital_write(pin, value):
    """
    digital write on pin from static pin name
    see pin.py module
    """
    _digital_write(pin[0], pin[1], value)

def _pin_mode(port, pin, direction):
    """__NATIVE__
    GPIO_TypeDef *port;
    uint16_t pin_mask;
    uint8_t pin_num;
    pPmObj_t pc;
    PmReturn_t retval = PM_RET_OK;

    NATIVE_SET_TOS(PM_NONE);

    if(NATIVE_GET_NUM_ARGS() != 3)
    {
      PM_RAISE(retval, PM_RET_EX_TYPE);
      return retval;
    }

    retval = _get_port_register(&port, &pin_mask, &pin_num);
    if(retval != PM_RET_OK)
      return retval;

    pc = NATIVE_GET_LOCAL(2);

    /* If the arg is not an integer, raise TypeError */
    if (OBJ_GET_TYPE(pc) != OBJ_TYPE_INT && OBJ_GET_TYPE(pc) != OBJ_TYPE_BOOL)
    {
      PM_RAISE(retval, PM_RET_EX_TYPE);
      return retval;
    }
    
    port->MODER  &= ~(GPIO_MODER_MODER0 << (pin_num * 2));
    port->MODER  |= (((pPmInt_t)pc)->val << (pin_num * 2));
    
    /* Speed mode configuration */
    port->OSPEEDR &= ~(GPIO_OSPEEDER_OSPEEDR0 << (pin_num * 2));
    port->OSPEEDR |= ((uint32_t)(GPIO_Speed_100MHz) << (pin_num * 2));
    
    return retval;
    """
    pass

def pin_mode(pin, direction):
    """
    configure pin mode from static pin name
    see pin.py module
    """
    _pin_mode(pin[0],pin[1],direction)
