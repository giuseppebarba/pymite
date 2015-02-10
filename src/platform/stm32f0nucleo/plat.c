/** @file
 * PyMite - A flyweight Python interpreter for 8-bit and larger microcontrollers.
 * Copyright 2002 Dean Hall.  All rights reserved.
 * PyMite is offered through one of two licenses: commercial or open-source.
 * See the LICENSE file at the root of this package for licensing details.
 *
 * some sections based on code (C) COPYRIGHT 2008 STMicroelectronics
 */

#undef __FILE_ID__
#define __FILE_ID__ 0x70

#include <stdio.h>
#include "stm32f0xx.h"
#include "stm32f0xx_hal_conf.h"
#include "pm.h"
#include "plat.h"

UART_HandleTypeDef UartHandle;
__IO ITStatus UartReady = RESET;


/**
  * @brief  This function is executed in case of error occurrence.
  * @param  None
  * @retval None
  */
static void Error_Handler(void)
{
	/* User may add here some code to deal with this error */
	while (1) {
	}
}

/*
 * Retargets the C library printf function to the USART.
 */
int fputc(int ch, FILE * f)
{
	if (HAL_UART_Transmit_IT(&UartHandle, (uint8_t *) ch, 1) != HAL_OK)
		Error_Handler();

	return ch;
}

/**
  * @brief  System Clock Configuration
  *         The system Clock is configured as follow :
  *            System Clock source            = PLL (HSI48)
  *            SYSCLK(Hz)                     = 48000000
  *            HCLK(Hz)                       = 48000000
  *            AHB Prescaler                  = 1
  *            APB1 Prescaler                 = 1
  *            HSI Frequency(Hz)              = 48000000
  *            PREDIV                         = 2
  *            PLLMUL                         = 2
  *            Flash Latency(WS)              = 1
  * @param  None
  * @retval None
  */
void SystemClock_Config(void)
{
	RCC_ClkInitTypeDef RCC_ClkInitStruct;
	RCC_OscInitTypeDef RCC_OscInitStruct;

	/* Select HSI48 Oscillator as PLL source */
	RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI48;
	RCC_OscInitStruct.HSI48State = RCC_HSI48_ON;
	RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
	RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI48;
	RCC_OscInitStruct.PLL.PREDIV = RCC_PREDIV_DIV2;
	RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL2;
	if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
		Error_Handler();

	/* Select PLL as system clock source and configure the HCLK and PCLK1 clocks dividers */
	RCC_ClkInitStruct.ClockType =
	    (RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_PCLK1);
	RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
	RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
	RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
	if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_1) != HAL_OK)
		Error_Handler();
}

PmReturn_t plat_init(void)
{
	/* STM32F0xx HAL library initialization:
	   - Configure the Flash prefetch
	   - Systick timer is configured by default as source of time base, but user
	   can eventually implement his proper time base source (a general purpose
	   timer for example or other time source), keeping in mind that Time base
	   duration should be kept 1ms since PPP_TIMEOUT_VALUEs are defined and
	   handled in milliseconds basis.
	   - Low Level Initialization
	 */
	HAL_Init();

	/* Configure LED2 */
	BSP_LED_Init(LED2);

	/* Configure the system clock to 48 MHz */
	SystemClock_Config();

	/*##-1- Configure the UART peripheral ###################################### */
	/* Put the USART peripheral in the Asynchronous mode (UART Mode) */
	/* UART configured as follows:
	   - Word Length = 8 Bits
	   - Stop Bit = One Stop bit
	   - Parity = None
	   - BaudRate = 9600 baud
	   - Hardware flow control disabled (RTS and CTS signals) */
	UartHandle.Instance = USARTx;
	UartHandle.Init.BaudRate = UART_BAUD;
	UartHandle.Init.WordLength = UART_WORDLENGTH_8B;
	UartHandle.Init.StopBits = UART_STOPBITS_1;
	UartHandle.Init.Parity = UART_PARITY_NONE;
	UartHandle.Init.HwFlowCtl = UART_HWCONTROL_NONE;
	UartHandle.Init.Mode = UART_MODE_TX_RX;
	UartHandle.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;
	if (HAL_UART_DeInit(&UartHandle) != HAL_OK)
		return PM_RET_ERR;

	if (HAL_UART_Init(&UartHandle) != HAL_OK)
		return PM_RET_ERR;

	return PM_RET_OK;
}

/* TODO: disable the peripherals and interrupts */
PmReturn_t plat_deinit(void)
{
	return PM_RET_OK;
}

/*
 * Gets a byte from the address in the designated memory space
 * Post-increments *paddr.
 */
uint8_t plat_memGetByte(PmMemSpace_t memspace, uint8_t const **paddr)
{
	uint8_t b = 0;

	switch (memspace) {
	case MEMSPACE_RAM:
	case MEMSPACE_PROG:
		b = **paddr;
		*paddr += 1;
		return b;
	case MEMSPACE_EEPROM:
	case MEMSPACE_SEEPROM:
	case MEMSPACE_OTHER0:
	case MEMSPACE_OTHER1:
	case MEMSPACE_OTHER2:
	case MEMSPACE_OTHER3:
	default:
		return 0;
	}
}

PmReturn_t plat_getByte(uint8_t * b)
{
	int c;
	PmReturn_t retval = PM_RET_OK;

	if (HAL_UART_Receive_IT(&UartHandle, (uint8_t *) & c, 1) != HAL_OK)
		Error_Handler();

	*b = c & 0xFF;
	if (c > 0xFF)
		PM_RAISE(retval, PM_RET_EX_IO);

	return retval;
}

PmReturn_t plat_putByte(uint8_t b)
{
	if (HAL_UART_Transmit_IT(&UartHandle, &b, 1) != HAL_OK)
		Error_Handler();

	return PM_RET_OK;
}

PmReturn_t plat_getMsTicks(uint32_t * r_ticks)
{
	/* TODO: make access atomic */
	*r_ticks = pm_timerMsTicks;

	return PM_RET_OK;
}

void plat_reportError(PmReturn_t result)
{
	switch (result) {
	case (PM_RET_EX):
		printf("PM_RET_EX: General exception\n");
		break;;
	case (PM_RET_EX_EXIT):
		printf("PM_RET_EX_EXIT: System exit\n");
		break;;
	case (PM_RET_EX_IO):
		printf("PM_RET_EX_IO: Input/output error\n");
		break;;
	case (PM_RET_EX_ZDIV):
		printf("PM_RET_EX_ZDIV: Zero division error\n");
		break;;
	case (PM_RET_EX_ASSRT):
		printf("PM_RET_EX_ASSRT: Assertion error\n");
		break;;
	case (PM_RET_EX_ATTR):
		printf("PM_RET_EX_ATTR: Attribute error\n");
		break;;
	case (PM_RET_EX_IMPRT):
		printf("PM_RET_EX_IMPRT: Import error\n");
		break;;
	case (PM_RET_EX_INDX):
		printf("PM_RET_EX_INDX: Index error\n");
		break;;
	case (PM_RET_EX_KEY):
		printf("PM_RET_EX_KEY: Key error\n");
		break;;
	case (PM_RET_EX_MEM):
		printf("PM_RET_EX_MEM: Memory error\n");
		break;;
	case (PM_RET_EX_NAME):
		printf("PM_RET_EX_NAME: Name error\n");
		break;;
	case (PM_RET_EX_SYNTAX):
		printf("PM_RET_EX_SYNTAX: Syntax error\n");
		break;;
	case (PM_RET_EX_SYS):
		printf("PM_RET_EX_SYS: System error\n");
		break;;
	case (PM_RET_EX_TYPE):
		printf("PM_RET_EX_TYPE: Type error\n");
		break;;
	case (PM_RET_EX_VAL):
		printf("PM_RET_EX_VAL: Value error\n");
		break;;
	case (PM_RET_EX_STOP):
		printf("PM_RET_EX_STOP: Stop iteration\n");
		break;;
	case (PM_RET_EX_WARN):
		printf("PM_RET_EX_WARN: Warning\n");
		break;;
	default:
		printf("???: unknown result value.\n");
		break;;
	};
	/* Print error */
	printf("Error:     0x%02X\n", result);
	printf("  Release: 0x%02X\n", gVmGlobal.errVmRelease);
	printf("  FileId:  0x%02X\n", gVmGlobal.errFileId);
	printf("  LineNum: %d\n", gVmGlobal.errLineNum);

	/* Print traceback */
	{
		pPmObj_t pframe;
		pPmObj_t pstr;
		PmReturn_t retval;

		printf("Traceback (top first):\n");

		/* Get the top frame */
		pframe = (pPmObj_t) gVmGlobal.pthread->pframe;

		/* If it's the native frame, print the native function name */
		if (pframe == (pPmObj_t) & (gVmGlobal.nativeframe)) {

			/* The last name in the names tuple of the code obj is the name */
			retval =
			    tuple_getItem((pPmObj_t) gVmGlobal.
					  nativeframe.nf_func->f_co->co_names,
					  -1, &pstr);
			if ((retval) != PM_RET_OK) {
				printf("  Unable to get native func name.\n");
				return;
			} else {
				printf("  %s() __NATIVE__\n",
				       ((pPmString_t) pstr)->val);
			}

			/* Get the frame that called the native frame */
			pframe = (pPmObj_t) gVmGlobal.nativeframe.nf_back;
		}

		/* Print the remaining frame stack */
		for (;
		     pframe != C_NULL;
		     pframe = (pPmObj_t) ((pPmFrame_t) pframe)->fo_back) {
			/* The last name in the names tuple of the code obj is the name */
			retval = tuple_getItem((pPmObj_t)
					       ((pPmFrame_t) pframe)->fo_func->
					       f_co->co_names, -1, &pstr);
			if ((retval) != PM_RET_OK)
				break;

			printf("  %s()\n", ((pPmString_t) pstr)->val);
		}
		printf("  <module>.\n");
	}
}
