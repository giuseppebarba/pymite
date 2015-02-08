# this module allow communication with I2C peripheral
# and STM32F4 board.
# this module just wrap the work of elias inside python
# method. Like every time python is a glue :)
# thx elias blog !
# reference : http://eliaselectronics.com/
# repository reference
# https://github.com/Torrentula/STM32F4-examples/blob/master/I2C%20Master/main.c
# 
# for the moment we only address I2C1 peripheral (this simplify argument work :)
# TODO : we should allow user to specify pin tuple.
#
# stm32F4 can have three peripheral I2C : I2C1, I2C2 and I2C3 as declared in
# stm32f4xx.h !
# for using I2C1 you could use two different pairs of pins :
#               (PB6 => SCL, PB7 => SDA)
#               (PB8 => SCL, PB9 => SDA)
#
# for using I2C2 you could use two different pairs of pins :
#               (PB10 => SCL, PB11 => SDA)
#               UNFINISHED
#
# for using I2C3 you could use two different pairs of pins :
#               UNFINISHED
#               UNFINISHED

"""__NATIVE__
#include "stm32f4xx_i2c.h"

void _init_I2C1(void){

	GPIO_InitTypeDef GPIO_InitStruct;
	I2C_InitTypeDef I2C_InitStruct;

	// enable APB1 peripheral clock for I2C1
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_I2C1, ENABLE);
	// enable clock for SCL and SDA pins
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB, ENABLE);

	/* setup SCL and SDA pins
	 * You can connect I2C1 to two different
	 * pairs of pins:
	 * 1. SCL on PB6 and SDA on PB7 
	 * 2. SCL on PB8 and SDA on PB9
	 */
	GPIO_InitStruct.GPIO_Pin = GPIO_Pin_6 | GPIO_Pin_7; // we are going to use PB6 and PB7
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_AF;			// set pins to alternate function
	GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;		// set GPIO speed
	GPIO_InitStruct.GPIO_OType = GPIO_OType_OD;			// set output to open drain --> the line has to be only pulled low, not driven high
	GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_UP;			// enable pull up resistors
	GPIO_Init(GPIOB, &GPIO_InitStruct);					// init GPIOB

	// Connect I2C1 pins to AF  
	GPIO_PinAFConfig(GPIOB, GPIO_PinSource6, GPIO_AF_I2C1);	// SCL
	GPIO_PinAFConfig(GPIOB, GPIO_PinSource7, GPIO_AF_I2C1); // SDA

	// configure I2C1 
	I2C_InitStruct.I2C_ClockSpeed = 100000; 		// 100kHz
	I2C_InitStruct.I2C_Mode = I2C_Mode_I2C;			// I2C mode
	I2C_InitStruct.I2C_DutyCycle = I2C_DutyCycle_2;	// 50% duty cycle --> standard
	I2C_InitStruct.I2C_OwnAddress1 = 0x00;			// own address, not relevant in master mode
	I2C_InitStruct.I2C_Ack = I2C_Ack_Disable;		// disable acknowledge when reading (can be changed later on)
	I2C_InitStruct.I2C_AcknowledgedAddress = I2C_AcknowledgedAddress_7bit; // set address length to 7 bit addresses
	I2C_Init(I2C1, &I2C_InitStruct);				// init I2C1

	// enable I2C1
	I2C_Cmd(I2C1, ENABLE);
}

/* This function issues a start condition and 
 * transmits the slave address + R/W bit
 * 
 * Parameters:
 * 		I2Cx --> the I2C peripheral e.g. I2C1
 * 		address --> the 7 bit slave address
 * 		direction --> the tranmission direction can be:
 * 						I2C_Direction_Tranmitter for Master transmitter mode
 * 						I2C_Direction_Receiver for Master receiver
 */
void _I2C_start(uint8_t address, uint8_t direction){
	// wait until I2C1 is not busy anymore
	while(I2C_GetFlagStatus(I2C1, I2C_FLAG_BUSY));
  
	// Send I2C1 START condition 
	I2C_GenerateSTART(I2C1, ENABLE);

	// wait for I2C1 EV5 --> Slave has acknowledged start condition
	while(!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_MODE_SELECT));

	// Send slave Address for write 
	I2C_Send7bitAddress(I2C1, address, direction);

	/* wait for I2C1 EV6, check if 
	 * either Slave has acknowledged Master transmitter or
	 * Master receiver mode, depending on the transmission
	 * direction
	 */ 
	if(direction == I2C_Direction_Transmitter){
		while(!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED));
	}
	else if(direction == I2C_Direction_Receiver){
		while(!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_RECEIVER_MODE_SELECTED));
	}
}

/* This function transmits one byte to the slave device
 * Parameters:
 *		I2Cx --> the I2C peripheral e.g. I2C1 
 *		data --> the data byte to be transmitted
 */
void _I2C_write(uint8_t data){
	I2C_SendData(I2C1, data);
	// wait for I2C1 EV8_2 --> byte has been transmitted
	while(!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_BYTE_TRANSMITTED));
}

/* This function reads one byte from the slave device 
 * and acknowledges the byte (requests another byte)
 */
uint8_t _I2C_read_ack(){
	// enable acknowledge of recieved data
	I2C_AcknowledgeConfig(I2C1, ENABLE);
	// wait until one byte has been received
	while( !I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_BYTE_RECEIVED) );
	// read data from I2C data register and return data byte
	uint8_t data = I2C_ReceiveData(I2C1);
	return data;
}

/* This function reads one byte from the slave device
 * and doesn't acknowledge the recieved data 
 */
uint8_t _I2C_read_nack(){
	// disabe acknowledge of received data
	I2C_AcknowledgeConfig(I2C1, DISABLE);
	// wait until one byte has been received
	while( !I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_BYTE_RECEIVED) );
	// read data from I2C data register and return data byte
	uint8_t data = I2C_ReceiveData(I2C1);
	return data;
}

/* This funtion issues a stop condition and therefore
 * releases the bus
 */
void _I2C_stop(){
	// Send I2C1 STOP Condition 
	I2C_GenerateSTOP(I2C1, ENABLE);
}
"""

# the slave address (example)
SLAVE_ADDRESS = 0x3D 
I2C_Direction_Transmitter  = 0x00
I2C_Direction_Receiver  = 0x01

def init_I2C1():
    """__NATIVE__
    PmReturn_t retval = PM_RET_OK;
    _init_I2C1();
    return retval;
    """
    pass

# This function issues a start condition and 
# transmits the slave address + R/W bit
# 
# Parameters:
#    I2Cx --> the I2C peripheral e.g. I2C1
#    address --> the 7 bit slave address
#    direction --> the tranmission direction can be:
#        I2C_Direction_Tranmitter for Master transmitter mode
#        I2C_Direction_Receiver for Master receiver
def I2C_start(address, direction):
    """__NATIVE__
    PmReturn_t retval = PM_RET_OK;

    pPmObj_t paddress;
    int32_t naddress;

    pPmObj_t pdirection;
    int32_t ndirection;

    uint8_t address;
    uint8_t direction;

    if(NATIVE_GET_NUM_ARGS() != 2)
    {
      PM_RAISE(retval, PM_RET_EX_TYPE);
      return retval;
    }

    paddress = NATIVE_GET_LOCAL(0);
    if (OBJ_GET_TYPE(paddress) != OBJ_TYPE_INT)
    {
      PM_RAISE(retval, PM_RET_EX_TYPE);
      return retval;
    }

    /* Raise ValueError if arg is not int within range(256) */
    naddress = ((pPmInt_t)paddress)->val;

    if ((naddress < 0) || (naddress > 255))
    {
        PM_RAISE(retval, PM_RET_EX_VAL);
        return retval;
    }
    address = (uint8_t)naddress;

    pdirection = NATIVE_GET_LOCAL(1);

    if (OBJ_GET_TYPE(pdirection) != OBJ_TYPE_INT)
    {
      PM_RAISE(retval, PM_RET_EX_TYPE);
      return retval;
    }
    ndirection = ((pPmInt_t)pdirection)->val;

    if ((ndirection < 0) || (ndirection > 255))
    {
        PM_RAISE(retval, PM_RET_EX_VAL);
        return retval;
    }
    direction = (uint8_t)ndirection;

    _I2C_start(address, direction);

    return retval;
    """
    pass

# This function transmits one byte to the slave device
# Parameters:
#     I2Cx --> the I2C peripheral e.g. I2C1 
#     data --> the data byte to be transmitted
# data should be an int !
def I2C_write(data):
    """__NATIVE__
    PmReturn_t retval = PM_RET_OK;
    pPmObj_t pdat;
    int32_t ndat;
    uint8_t data;

    if(NATIVE_GET_NUM_ARGS() != 1)
    {
      PM_RAISE(retval, PM_RET_EX_TYPE);
      return retval;
    }

    pdat = NATIVE_GET_LOCAL(0);
    if (OBJ_GET_TYPE(pdat) != OBJ_TYPE_INT)
    {
      PM_RAISE(retval, PM_RET_EX_TYPE);
      return retval;
    }

    ndat = ((pPmInt_t)pdat)->val;
    if ((ndat < 0) || (ndat > 255))
    {
        PM_RAISE(retval, PM_RET_EX_VAL);
        return retval;
    }
    data = (uint8_t)ndat;

    _I2C_write(data);

    return retval;
    """
    pass

# This function reads one byte from the slave device 
# and acknowledges the byte (requests another byte)
def I2C_read_ack():
    """__NATIVE__
    PmReturn_t retval = PM_RET_OK;
    pPmObj_t p0;

    uint8_t data = _I2C_read_ack();

    // build python output
    retval = int_new( *(uint32_t *)(data), &p0);
    NATIVE_SET_TOS(p0);
    return retval;
    """

# This function reads one byte from the slave device
# and doesn't acknowledge the recieved data 
def I2C_read_nack():
    """__NATIVE__
    PmReturn_t retval = PM_RET_OK;
    pPmObj_t p0;

    uint8_t data = _I2C_read_nack();

    // build python output
    int_new( *(uint32_t *)(data) , &p0);
    NATIVE_SET_TOS(p0);
    return retval;
    """
    pass

# This funtion issues a stop condition and therefore
# releases the bus
def I2C_stop():
    """__NATIVE__
    PmReturn_t retval = PM_RET_OK;
    _I2C_stop();
    return retval;
    """
    pass

def test(SLAVE_ADDRESS):
    """
    """
    init_I2C1()
    received_data=[]

    while True :
        # start a transmission in Master transmitter mode
        I2C_start(SLAVE_ADDRESS, I2C_Direction_Transmitter)
        # write one byte to the slave
        I2C_write(0x20)
        # write another byte to the slave
        I2C_write(0x03) 
        # stop the transmission
        I2C_stop()
        
        # start a transmission in Master receiver mode
        I2C_start(SLAVE_ADDRESS, I2C_Direction_Receiver)
        # read one byte and request another byte
        received_data[0] = I2C_read_ack()
        # read one byte and don't request another byte
        received_data[1] = I2C_read_nack()
        # stop the transmission
        I2C_stop()
        print received_data

# :mode=c:
