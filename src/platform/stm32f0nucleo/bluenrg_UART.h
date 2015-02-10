#ifndef _BLUENRG_INIT_H_
#define _BLUENRG_INIT_H_

#include "stm32f0xx_hal.h"
#include "stm32f0xx_nucleo.h"
#include "ble_status.h"
#include "pm.h"

PmReturn_t BTLE_UART_init(void);
PmReturn_t BTLE_UART_send(const uint8_t *data, uint8_t length);

#endif /* _BLUENRG_INIT_H_ */
