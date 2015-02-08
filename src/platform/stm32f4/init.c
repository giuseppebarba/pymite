// 
// STM32F4dxx 基本的な設定を行うプログラム テンプレート
//  初期設定プログラム(RCC,USART2,TIM4,SysTick)
// 参考資料：
//	RM0090 Reference manual - STM32F4xx advanced ARM-based 32-bit MCUs
//	PM0056 Programming manual - STM32Fxxx Cortex-M3 programming manual
//	UM1472 Users manual - STM32F4DISCOVERY discovery board
// 
#include "stm32f4xx.h"
#include "core_cm4.h"
#include "sysclk_config.h"

//	TIM4を使ったPWMはSOFT_PWMを#undefすること。
// #define SOFT_PWM

//
// メモリユティリティサブルーチン
//
static void *init_memset(void *dst,int c,long cnt){
unsigned char *p;
	p= (unsigned char *)dst;
	for(;cnt;cnt--){
		*p= (unsigned char)c;
		p++;
	};
	return(dst);
}
static void *init_memmove(void *dst,void *src,long cnt){
unsigned char *p,*q;
	if(0==cnt || src==dst)return(dst);
	if(src>dst){
		p= (unsigned char *)dst;
		q= (unsigned char *)src;
		for(;cnt;cnt--){
			*p= *q;
			p++;q++;
		};
	}else{
		p= (unsigned char *)((long)dst+cnt);
		q= (unsigned char *)((long)src+cnt);
		for(;cnt;cnt--){
			--p;--q;
			*p= *q;
		};
	};
	return(dst);
}
static void *init_memcpy(void *dst,void *src,long cnt){
unsigned char *p,*q;
	if(0==cnt || src==dst)return(dst);
	p= (unsigned char *)dst;
	q= (unsigned char *)src;
	for(;cnt;cnt--){
		*p= *q;
		p++;q++;
	};
	return(dst);
}

//
// リンカスクリプトで定義されているデータセクションのラベル
//
extern unsigned char _sidata[];
extern unsigned char _sdata[];
extern unsigned char _edata[];
extern unsigned char _sbss[];
extern unsigned char _ebss[];
extern unsigned char _endof_sram[];

//
// Reset and Clock Controller initialize
// 無難なように未使用pinをpullUpで入力にするようGPIOA..Eにクロックを供給する指定をしている
//
void init_RCC(void){
#define default_RCC_CR (RCC_CR_HSION | RCC_CR_HSITRIM_4)	// 0x0000xx83
#define default_RCC_CFGR (0)	// 0x00000000
#define default_RCC_CIR (0) 	// 0x00000000
#define default_RCC_AHB1ENR (RCC_AHB1ENR_CCMDATARAMEN)  	// 0x00100000
#define default_RCC_AHB2ENR (0)	// 0x00000000
#define default_RCC_AHB3ENR (0)	// 0x00000000
#define default_RCC_APB1ENR (0)	// 0x00000000
#define default_RCC_APB2ENR (0)	// 0x00000000
#define user_RCC_CFGR ((CFGR_PPRE2 << 13) | (CFGR_PPRE1 << 10) | (CFGR_HPRE << 4))

#ifdef SysClockSource_HSE_Xtal
	RCC->CR = RCC_CR_HSEON | default_RCC_CR;	// crystal osc. and ceramic res.
#else
#	ifdef SysClockSource_HSE_EXT
		RCC->CR = (RCC_CR_HSEBYP | RCC_CR_HSEON) | default_RCC_CR;	// EXT_CLOCK in
#	else
		RCC->CR = default_RCC_CR;	// only HSI
#	endif
#endif

	RCC->CFGR= user_RCC_CFGR;	// reset CFGR
	RCC->CIR= default_RCC_CIR;  	// disable all interrupts

#ifdef ClockSource_is_HSE
	while(0==(RCC->CR & RCC_CR_HSERDY));	// wait for HSE ready to stable
	RCC->CFGR= user_RCC_CFGR | RCC_CFGR_SW_HSE ;
	while(RCC_CFGR_SWS_HSE!=(RCC->CFGR & RCC_CFGR_SWS));
// error description in REF.man correct_default_value=0x02403010
#	define default_RCC_PLLCFGR (RCC_PLLCFGR_PLLQ_1 | RCC_PLLCFGR_PLLSRC_HSE | RCC_PLLCFGR_PLLN_7 | RCC_PLLCFGR_PLLN_6 | RCC_PLLCFGR_PLLM_4 ) // 0x02403010
#	define user_RCC_PLLCFGR (calcRCC_PLLCFG | RCC_PLLCFGR_PLLSRC_HSE )
#else
	while(0==(RCC->CR & RCC_CR_HSIRDY));	// wait for HSI ready to stable
	RCC->CFGR= user_RCC_CFGR | RCC_CFGR_SW_HSI ;
	while(RCC_CFGR_SWS_HSI!=(RCC->CFGR & RCC_CFGR_SWS));
#	define default_RCC_PLLCFGR (RCC_PLLCFGR_PLLQ_1 | RCC_PLLCFGR_PLLN_7 | RCC_PLLCFGR_PLLN_6 | RCC_PLLCFGR_PLLM_4 ) // 0x02003010
#	define user_RCC_PLLCFGR (calcRCC_PLLCFG)
#endif

	FLASH->ACR= ( FLASH_ACR_PRFTEN | FLASH_ACR_ICEN | FLASH_ACR_DCEN | FLASH_ACR_ICRST | FLASH_ACR_DCRST );

#ifdef SysClockPLL_USE
	FLASH->ACR |= FlashReadLatency;
	RCC->CFGR = user_RCC_CFGR;
	RCC->PLLCFGR = user_RCC_PLLCFGR;
	RCC->CR |= RCC_CR_PLLON;            	// enable PLL
	while(0==(RCC->CR & RCC_CR_PLLRDY));	// wait for PLL ready to stable
	RCC->CFGR|= RCC_CFGR_SW_PLL | RCC_CFGR_MCO1_1 | RCC_CFGR_MCO1_0; // MCO2 is SYSCLK MCO1 is PLL
	while(RCC_CFGR_SWS_PLL!=(RCC->CFGR & RCC_CFGR_SWS));

#endif

// supply clock for the peripheral buses
	RCC->AHB1ENR = default_RCC_AHB1ENR;
	RCC->AHB2ENR = default_RCC_AHB2ENR;
	RCC->AHB3ENR = default_RCC_AHB3ENR;
	RCC->APB1ENR = default_RCC_APB1ENR;
	RCC->APB2ENR = default_RCC_APB2ENR;

//
//	GPIOA..Eにクロックを注入する
	RCC->AHB1ENR |= RCC_AHB1ENR_GPIOAEN | RCC_AHB1ENR_GPIOBEN | RCC_AHB1ENR_GPIOCEN | RCC_AHB1ENR_GPIODEN | RCC_AHB1ENR_GPIOEEN;
}

//
//
//
void init_GPIO(void);   // initializing GPIO and call init_TIM4().
void init_TIM4(void);   // initializing and start count.
void init_USART2(void); // initializing but not start communication.


void init_GPIO(void){
#define default_GPIOA_MODER (GPIO_MODER_MODER15_1 | GPIO_MODER_MODER14_1 | GPIO_MODER_MODER13_1)
#define default_GPIOB_MODER (GPIO_MODER_MODER4_1 | GPIO_MODER_MODER3_1 )
//
// GPIOA configure
	GPIOA->PUPDR = 0x00005556; // GPIOA_15..9:input GPIOA_7..1:input_PullUp GPIOA_0:input_PullDown
// PA0=USER_BTN PA2=USART2_Tx PA3=USART2_Rx PA8=MCO1
// PA15..13=JTAG(SWD)
	GPIOA->MODER = default_GPIOA_MODER | GPIO_MODER_MODER8_1 | GPIO_MODER_MODER3_1 | GPIO_MODER_MODER2_1 ;
	GPIOA->ODR = 0;
	GPIOA->AFR[0]= 0x00007700; // GPIOA3,2 as USART2_Rx,Tx
	GPIOA->AFR[1]= 0x00000000; // GPIOA8 as MCO1

//
// GPIOB configure
	GPIOB->PUPDR = 0x55555415; // GPIOB_15..5,2..0:input_PullUp
// PB4..3=JTAG(SWD)
	GPIOB->MODER =  default_GPIOB_MODER ;
	GPIOB->ODR = 0;

//
// GPIOC configure
	GPIOC->PUPDR = 0x05515555; // GPIOC_13..0:input_PullUp(except PC9)
// PC15..14=LSE_Xtal PC9=MCO2
	GPIOC->MODER = GPIO_MODER_MODER9_1;
	GPIOC->ODR = 0;
	GPIOC->AFR[0]= 0x00000000; // default
	GPIOC->AFR[1]= 0x00000000; // GPIOC9 as MCO1

//
// GPIOD configure
	GPIOD->PUPDR = 0x00555555; // GPIOD_11..0:input_PullUp
#ifdef SOFT_PWM
// LED_15=BL,14=RE,13=OR,12=GR is output
// default I/O_LEVEL=L
	GPIOD->MODER = (GPIO_MODER_MODER15_0 | GPIO_MODER_MODER14_0 | GPIO_MODER_MODER13_0 | GPIO_MODER_MODER12_0);
	GPIOD->ODR = 0;
#else
// GPIOD_15..12 set alternate function mode (TIM4 = AF2)
	GPIOD->MODER = (GPIO_MODER_MODER15_1 | GPIO_MODER_MODER14_1 | GPIO_MODER_MODER13_1 | GPIO_MODER_MODER12_1);
	GPIOD->AFR[1]= 0x22220000; // DPIOD_15..12 as TIM4_CH4..CH1
	init_TIM4();
#endif /* SOFT_PWM */

//
// GPIOE configure
	GPIOE->PUPDR = 0x55555555; // GPIOE_15..0:input_PullUp
//
	GPIOE->MODER = 0;
	GPIOE->ODR = 0;

}


void init_TIM4(void){
//	TIM4にクロックを注入する
	RCC->APB1ENR |= RCC_APB1ENR_TIM4EN;
//	TIM4 configure
//	TIM4_CH4=BL,3=RE,2=OR,1=GR is output PWM-edgeAlign mode
#define	default_TIMx_CR1	(0)
#define	default_TIMx_CR2	(0)
#define	CCER_SET(x) 1<<(4*(x))
TIM4->CR1= default_TIMx_CR1; // reset value
TIM4->CR2= default_TIMx_CR2; // reset value

TIM4->ARR= 255; // auto reload 255 as like a 8bit-timer :)
TIM4->CCR1= TIM4->CCR2= TIM4->CCR3= TIM4->CCR4= 0; // clear compair value(PWM OFF)
TIM4->CCER=  TIM_CCER_CC4E | TIM_CCER_CC3E | TIM_CCER_CC2E | TIM_CCER_CC1E; // enable output all channels
TIM4->CCMR1= TIM_CCMR1_OC2M_2 | TIM_CCMR1_OC2M_1 | TIM_CCMR1_OC1M_2 | TIM_CCMR1_OC1M_1;
TIM4->CCMR2= TIM_CCMR2_OC4M_2 | TIM_CCMR2_OC4M_1 | TIM_CCMR2_OC3M_2 | TIM_CCMR2_OC3M_1;
TIM4->CNT= 0;
TIM4->SR=  0;
TIM4->CR1= TIM_CR1_CEN; // f/1,PWM-edgeAligned,upcount,countEnable
TIM4->BDTR = TIM_BDTR_MOE;
}


void init_USART2(void){
//	USART2にクロックを注入する
	RCC->APB1ENR |= RCC_APB1ENR_USART2EN;
//	USART2 configure( deinitialize only use start_USART2() )
	USART2->CR1= 0;
	USART2->CR2= 0;
	USART2->CR3= 0;
	USART2->SR= 0xc0;
}


volatile uint32_t CNT_tick;
volatile uint32_t fCNT_tick;
volatile uint32_t dCNT_tick[4];

extern int main(void);


void init_peripherals(void){
// reset & clock configure
	init_RCC();

// GPIO configure
	init_GPIO();

// set up sysTick facility
// 1ms ごとにsysTickイベントを発生させる
//
#define user_SysTick_CTRL (SysTick_CTRL_CLKSOURCE_Msk | SysTick_CTRL_TICKINT_Msk | SysTick_CTRL_ENABLE_Msk)
//  This function initialises the system tick timer and its interrupt and start the system tick timer.
//  Counter is in free running mode to generate periodical interrupts.
	SysTick_Config(CPUClock / 1000); // SysTick event every a milli-second
	for(CNT_tick=sizeof(dCNT_tick)/sizeof(dCNT_tick[0]);0<CNT_tick;){
		--CNT_tick;
		dCNT_tick[CNT_tick]=0;
	};

// USART2 configure
	init_USART2();
}


void hardware_init_hook(void){
// configure cortex-M4 core facility
// set vector table
#ifdef VECTOR_ON_SRAM
	SCB->VTOR = SRAM_BASE;	// vector table from top of SRAM
#else
	SCB->VTOR = FLASH_BASE;	// vector table from top of FLASH
#endif
#if (__FPU_PRESENT == 1) && (__FPU_USED == 1)
	SCB->CPACR |= ((3UL<<(10*2))|(3UL<<(11*2))); // set enable CP10&CP11 access
#endif /* FPU */
};

void software_init_hook(void){
//	__libc_init_array();
//	__libc_fini_array();
}

void init(void){
	hardware_init_hook();

// .data and .bss,.COMM area initializing
	init_memcpy(_sdata, _sidata, _edata - _sdata);	// set initial values onto .data 
	init_memset(_sbss, 0, _ebss - _sbss);	// clear .bss, .COMM

	init_peripherals();

	software_init_hook();
};

extern void start_USART2(int);
extern struct {
	uint8_t err_flag;
	uint8_t not_done;
	char tx_data;
	char rx_data;
} usart2; // IGNORE trailer thing...

//
// リセット後に実行されるルーチン(vector.cでspとstartのエントリーを指定している)
//
void start(void){
// spや周辺のレジスタがおかしくなってる可能性もあるからリセットのほうがいいなコ↓コ↑
	loop{
		init();

		if(1==(GPIOA->IDR & 1)){ // is push USER BUTTON?
			start_USART2(UART_ALT_BAUD);
		}else{
			start_USART2(UART_BAUD);
		};
		EI;

		dCNT_tick[0]=10; // wait for 10ms for stabilizing USART clock generator
		while(dCNT_tick[0]);

		main();

		dCNT_tick[0]=10000; // 10000ms wait limit
		while( dCNT_tick[0] && usart2.not_done );
		DI;
	};
}


//
// sysTickイベントごとにtickカウンタをインクリメントする(vector.cで指定している)
//
void hndl_SysTick(void)	{	// 1000Hz system tick count and go around
uint32_t i;
	SysTick->CTRL= user_SysTick_CTRL; // clear all status flag quickly
	CNT_tick++;
	fCNT_tick++;
	for(i=sizeof(dCNT_tick)/sizeof(dCNT_tick[0]);0<i;){
		--i;
		if(0<dCNT_tick[i])dCNT_tick[i]--;
	};
}

