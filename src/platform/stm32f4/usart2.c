// 
// STM32F4xx I/O プログラム
//  USART2 関係 (ピン出力定義はinit.cで行っている)
// 参考資料：
//	RM0090 Reference manual - STM32F4xx advanced ARM-based 32-bit MCUs
//	PM0056 Programming manual - STM32Fxxx Cortex-M3 programming manual
//	UM1472 Users manual - STM32F4DISCOVERY discovery board
// 
#include "stm32f4xx.h"
#include "core_cm4.h"
#include "sysclk_config.h"

extern void init_USART2(void);
extern void USART2_init(int brr);

#define	INT_pos	(USART2_IRQn)

void start_USART2(int brr){
	init_USART2();  	// enable RCC_USART2 , reset USART2_CR1..CR3
	USART2_init(brr);	// initialize USART2
}

#define	SCI	USART2
#define	sci	usart2
#define	BUFFER_SIZE	(256)
// #define	ENTER_hndl	{TIM4->CCR4=0xFF;}
// #define	EXIT_hndl	{TIM4->CCR4=0x00;}
#define	ENTER_hndl
#define	EXIT_hndl

#define	SCI_init	USART2_init
#define	SCI_putc_direct	USART2_putc_direct
#define	SCI_puts_direct	USART2_puts_direct
#define	SCI_nbuf	USART2_nbuf
#define	SCI_getc	USART2_getc
#define	SCI_putc	USART2_putc
#define	SCI_puts	USART2_puts
#define	SCI_write	USART2_write
#define	SCI_writes	USART2_writes

#include "usart.h"

