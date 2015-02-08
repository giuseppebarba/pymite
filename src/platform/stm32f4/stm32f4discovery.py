# This file is Copyright 2012 H.I.
#
## @file
#  @copybrief stm32f4discovery
#
## @package stm32f4discovery
#  @brief stm32f4discovery Access Module
#
# Provides generic access to the stm32f4discovery Eval-kit board.
#
# <b>USAGE</b>
#
# \code
# import stm32f4discovery
# 
# a=stm32f4discovery.BTN()      # 1=USER BUTTON is PUSH, 0=not PUSH
# stm32f4discovery.LED(0, 100)  # first arg sel_LED 0:LED_GREEN 1:LED_ORANGE 2:LED_RED 3:LED_BLUE
#                               # second arg LED brightness(0:off >255:full)
# \endcode
# 

"""__NATIVE__
#include "stm32f4xx.h"
#include "core_cm4.h"
#include "sysclk_config.h"

#include "pm.h"

/*
 * I/O register operations
 */
PmReturn_t _stm32f4_IOREG(void){ // read operation if one arg and read/write operation if two args
	PmReturn_t retval = PM_RET_OK;
	pPmObj_t p0,p1,p2;

	uint32_t addr;

	switch(NATIVE_GET_NUM_ARGS()){
	  case( 1 ):
		p0= NATIVE_GET_LOCAL(0);
		if(OBJ_GET_TYPE(p0) != OBJ_TYPE_INT){
			PM_RAISE(retval, PM_RET_EX_TYPE);
			break;;
		};
		addr= PERIPH_BASE + ((pPmInt_t)p0)->val;
		int_new( *(uint32_t *)(addr) , &p2);

		NATIVE_SET_TOS( p2 );
		break;;

	  case( 2 ):
		p0= NATIVE_GET_LOCAL(0);
		if(OBJ_GET_TYPE(p0) != OBJ_TYPE_INT){
			PM_RAISE(retval, PM_RET_EX_TYPE);
			break;;
		};

		p1= NATIVE_GET_LOCAL(1);
		if(OBJ_GET_TYPE(p1) != OBJ_TYPE_INT){
			PM_RAISE(retval, PM_RET_EX_TYPE);
			break;;
		};

		addr= PERIPH_BASE + ((pPmInt_t)p0)->val;
		int_new( *(uint32_t *)(addr) , &p2);
		NATIVE_SET_TOS( p2 );
		*(uint32_t *)(addr)= (uint32_t)(((pPmInt_t)p1)->val);
		break;;

	  default:
		PM_RAISE(retval, PM_RET_EX_TYPE);
		break;;
	};
	PM_REPORT_IF_ERROR(retval);
	return(retval);
}

PmReturn_t _stm32f4_IOWRT(void){ // write only operation
	PmReturn_t retval = PM_RET_OK;
	pPmObj_t p0,p1;

	uint32_t addr;

	switch(NATIVE_GET_NUM_ARGS()){
	  case( 2 ):
		p0= NATIVE_GET_LOCAL(0);
		if(OBJ_GET_TYPE(p0) != OBJ_TYPE_INT){
			PM_RAISE(retval, PM_RET_EX_TYPE);
			break;;
		};

		p1= NATIVE_GET_LOCAL(1);
		if(OBJ_GET_TYPE(p1) != OBJ_TYPE_INT){
			PM_RAISE(retval, PM_RET_EX_TYPE);
			break;;
		};

		addr= PERIPH_BASE + ((pPmInt_t)p0)->val;

		NATIVE_SET_TOS( PM_NONE );
		*(uint32_t *)(addr)= (uint32_t)(((pPmInt_t)p1)->val);
		break;;

	  default:
		PM_RAISE(retval, PM_RET_EX_TYPE);
		break;;
	};
	PM_REPORT_IF_ERROR(retval);
	return(retval);
}

PmReturn_t _do_RAISE(void){
	PmReturn_t retval = PM_RET_OK;
	pPmObj_t p0;

	switch(NATIVE_GET_NUM_ARGS()){
	  case( 0 ):
		PM_RAISE(retval, PM_RET_EX_TYPE);
		break;;

	  case( 1 ):
		p0= NATIVE_GET_LOCAL(0);
		if(OBJ_GET_TYPE(p0) == OBJ_TYPE_INT){
			PM_RAISE(retval,  ((pPmInt_t)p0)->val & 0xff);
		}else{
			PM_RAISE(retval, PM_RET_EX_TYPE);
		};

	  default:
		PM_RAISE(retval, PM_RET_EX_TYPE);
		break;;
	};
	PM_REPORT_IF_ERROR(retval);
	return(retval);
}

PmReturn_t _do_sense_USER_BTN(void){
	pPmObj_t p0;
	int_new( (int32_t)(GPIOA->IDR & 1) , &p0);
	NATIVE_SET_TOS( p0 );
	return( (PmReturn_t)PM_RET_OK );
}



#define SPI1_ena_CS { GPIOE->BSRRH =  0x0008; } // PE3=Lo
#define SPI1_dis_CS { GPIOE->BSRRL =  0x0008; } // PE3=Hi
#define AXEL_CMD_WRITE (0x00)
#define AXEL_CMD_READ  (0x80)
#define AXEL_CMD_SGL   (0x00)
#define AXEL_CMD_MLT   (0x80)
#define AXEL_CR1           (0x20)
#define AXEL_STATUS_REG    (0x27)
#define AXEL_OUT_X         (0x29)
#define AXEL_OUT_Y         (0x2B)
#define AXEL_OUT_Z         (0x2D)
uint8_t _send_recv_SPI1(uint8_t c){
	if(0 == ((SPI1->CR1) & SPI_CR1_SPE)){
		return(0xff);
	}else{
		while(SPI_SR_TXE != ((SPI1->SR) & (SPI_SR_TXE)));
		SPI1->DR= c;
		while(SPI_SR_RXNE != ((SPI1->SR) & (SPI_SR_RXNE)));
		return(SPI1->DR);
	};
}
void _write_AXEL(uint8_t REG, uint8_t data){
	SPI1_ena_CS;
	_send_recv_SPI1( (REG &0x3f) | AXEL_CMD_WRITE | AXEL_CMD_SGL); // Write Single to REG addr.
	_send_recv_SPI1(data);
	SPI1_dis_CS;
}
uint8_t _read_AXEL(uint8_t REG){
uint8_t data;
	SPI1_ena_CS;
	_send_recv_SPI1( (REG & 0x3f) | AXEL_CMD_READ | AXEL_CMD_SGL);  // Read Single from REG addr
	data= _send_recv_SPI1(0xff);
	SPI1_dis_CS;
	return(data);
}
PmReturn_t _deconfig_port_for_SPI1_AXEL(void){
	RCC->APB2RSTR=  RCC_APB2RSTR_SPI1RST; // SPI1 unit reset
	NATIVE_SET_TOS(PM_NONE);
	return( (PmReturn_t)PM_RET_OK );
}
PmReturn_t _config_port_for_SPI1_AXEL(void){
// set RCC to enable clock SPI1(APB2) and reset
	RCC->APB2RSTR=  RCC_APB2RSTR_SPI1RST; // SPI1 unit reset

// set SPImode AXEL sensor 
	GPIOE->MODER &= 0xffffff00; // set PE3..0 is input
	GPIOE->MODER |= 0x00000040; // set PE3 is output
	SPI1_dis_CS;

	RCC->APB2RSTR&= ~RCC_APB2RSTR_SPI1RST;// SPI1 unit reset release

// set SPI1 port config onto PA7..5
	GPIOA->MODER &= 0xffff03ff; // set PA7..5 is input
	GPIOA->AFR[0] &= 0x000fffff; // set PA7..5 is AF0
	GPIOA->AFR[0] |= (5<<28)|(5<<24)|(5<<20); // set PA7..5 is AF5
	GPIOA->MODER |= (GPIO_MODER_MODER7_1 | GPIO_MODER_MODER6_1 | GPIO_MODER_MODER5_1); // set PA7..5 is AlterFunc

	RCC->APB2ENR |= RCC_APB2ENR_SPI1EN;
// setup SPI1
//		 biDir/SendE/SPIE/9bit/MSBfirst/(APB2/8=2)/Master/SPImode3
	SPI1->CR2 = 0;
	SPI1->SR  = 0;
	SPI1->CR1 = SPI_CR1_SSM | SPI_CR1_SSI | SPI_CR1_SPE | (2<<3) | SPI_CR1_MSTR | SPI_CR1_CPOL | SPI_CR1_CPHA;

	SPI1_ena_CS;
	_write_AXEL(AXEL_CR1, 0x47); // write powerup to CR1
	NATIVE_SET_TOS(PM_NONE);
	SPI1_dis_CS;

	return( (PmReturn_t)PM_RET_OK );
}
PmReturn_t _AXEL_out(void){
	PmReturn_t retval = PM_RET_OK;
	pPmObj_t p0;

	if(SPI1->CR1 & SPI_CR1_SPE){
		retval = tuple_new(4, &p0); // create a tupple in four elements
		PM_RETURN_IF_ERROR(retval);
	
		/* Put the thress axel_value and status in the tuple p0 */
		int_new( (int8_t)_read_AXEL(AXEL_OUT_X) , &(((pPmTuple_t)p0)->val[0]));
		int_new( (int8_t)_read_AXEL(AXEL_OUT_Y) , &(((pPmTuple_t)p0)->val[1]));
		int_new( (int8_t)_read_AXEL(AXEL_OUT_Z) , &(((pPmTuple_t)p0)->val[2]));
		int_new( _read_AXEL(AXEL_STATUS_REG) , &(((pPmTuple_t)p0)->val[3]));
		NATIVE_SET_TOS( p0 );
		return( (PmReturn_t)PM_RET_OK );
	}else{
		NATIVE_SET_TOS( PM_NONE );
		return( (PmReturn_t)PM_RET_OK );
	};

}
PmReturn_t _AXEL_REG(void){
	PmReturn_t retval = PM_RET_OK;
	pPmObj_t p0,p1,p2;

	switch(NATIVE_GET_NUM_ARGS()){
	  case( 1 ):
		p0= NATIVE_GET_LOCAL(0);
		if(OBJ_GET_TYPE(p0) == OBJ_TYPE_INT){
			int_new( _read_AXEL( ((pPmInt_t)p0)->val)  , &p0 );
			NATIVE_SET_TOS( p0 );
		}else{
			PM_RAISE(retval, PM_RET_EX_TYPE);
		};
		break;;
	  case( 2 ):
		p0= NATIVE_GET_LOCAL(0);
		if(OBJ_GET_TYPE(p0) == OBJ_TYPE_INT){
			p1= NATIVE_GET_LOCAL(1);
			if(OBJ_GET_TYPE(p1) == OBJ_TYPE_INT){
				int_new( _read_AXEL( ((pPmInt_t)p0)->val) , &p2 );
				NATIVE_SET_TOS( p2 );
				_write_AXEL( ((pPmInt_t)p0)->val , ((pPmInt_t)p1)->val );
			}else{
				PM_RAISE(retval, PM_RET_EX_TYPE);
			};
		}else{
			PM_RAISE(retval, PM_RET_EX_TYPE);
		};
		break;;
	  default:
		PM_RAISE(retval, PM_RET_EX_TYPE);
		break;;
	};
	PM_REPORT_IF_ERROR(retval);
	return(retval);
}

"""

def __rise_TypeError(a=0xee):
	"""__NATIVE__
	return	_do_RAISE();
	"""
	pass;

def IOREG(parm,val): # parm address_offset from PERIPH_BASE(0x40000000) and value(long)
	"""__NATIVE__
	return _stm32f4_IOREG();
	"""
	pass

def IOWRT(parm,val): # parm address_offset from PERIPH_BASE(0x40000000) and value(long)
	"""__NATIVE__
	return _stm32f4_IOWRT();
	"""
	pass

def BTN():
## AHB1=0x020000
#	_GPIOA_IDR= 0x020000 + 0x0000 + 0x10; # // offset from BASE(0x40000000)
#	return (IOREG(_GPIOA + _IDR) & 1);
	"""__NATIVE__
	return ( _do_sense_USER_BTN() );
	"""
	pass;

def LED(n, val=0):
# APB1=0x000000
	_TIM4_CCR=  0x000000 + 0x0800 + 0x34; # // offset from BASE(0x40000000)
	if (0>n) or (3<n):
		__rise_TypeError();
	IOREG( _TIM4_CCR + (n * 4) , val);

def _deconf_AXEL():
	"""__NATIVE__
	return _deconfig_port_for_SPI1_AXEL();
	"""
	pass;

def _conf_AXEL():
	"""__NATIVE__
	return _config_port_for_SPI1_AXEL();
	"""
	pass;

def _conf_AXEL():
	"""__NATIVE__
	return _config_port_for_SPI1_AXEL();
	"""
	pass;

def _get_AXEL():
	"""__NATIVE__
	return _AXEL_out();
	"""
	pass;

def AXEL(x=None):
	if(0==x):
		return _deconf_AXEL();
	elif(1==x):
		return _conf_AXEL();
	elif(None==x):
		return _get_AXEL();
	else:
		__rise_TypeError();

def AXEL_REG(addr,parm):
	"""__NATIVE__
	return _AXEL_REG();
	"""
	pass;
# :mode=c:
