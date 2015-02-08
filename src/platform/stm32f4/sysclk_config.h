#ifndef SYSCLK_CONFIG_H
#define SYSCLK_CONFIG_H
// 
// STM32F4discoveryボードのクロック定義
// 参考資料：
//	RM0090 Reference manual - STM32F4xx advanced ARM-based 32-bit MCUs
//	PM0056 Programming manual - STM32Fxxx Cortex-M3 programming manual
//	UM1472 Users manual - STM32F4DISCOVERY discovery board
// 

//	STM32F4discoveryは，外部のクロックソースを使う場合に，そのままだと
//	ST-linkコントローラーのMCO出力がOSC_INに接続されている。
//	そのままの場合は，SysClockSource_HSE_EXTを定義する。
//	STM32F4側に実装されているXtalをクロックソースとして使用するならば
//	ハンダ面にあるR68(100ohm)抵抗を外して(できればSB15&16をオープンに)
//	Xtal-OSCを有効にしてからSysClockSource_HSE_Xtalを定義する。
//	SysClockSource_HSE_*が定義されてなければ，内蔵HSI(16MHz)を使う。
#define SysClockSource_HSE_Xtal
//#define SysClockSource_HSE_EXT

#if defined(SysClockSource_HSE_Xtal) || defined(SysClockSource_HSE_EXT)
#	define ClockSource_is_HSE 1
#endif

#ifndef SysClock
#	ifdef ClockSource_is_HSE
#		define SysClock (8000000)	// assumed external clock source is 8MHz
#	else
#		define SysClock (16000000)	// HSI=16MHz
#	endif
#endif

#define SysClockPLL_USE	// HCLK= SysClock x PLL_N / PLL_M / PLL_P
// AHB= 29.5MHz= 8MHz x 118 / 16 / 2  =APB2 =APB1
// AHB= 118MHz = 16MHz x 118 / 8 / 2  =2xAPB2 =4xAPB1

#define SysClockPLL_N (236)	// default=192 valid_range=64..432
#define SysClockPLL_M (8)	// default=16  valid_range=2..63
#define SysClockPLL_Q (2)	// default=2   valid_range=2..15
#define SysClockPLL_P (2)	// default=2   valid_range=2,4,6,8

//#  define FlashReadLatencyFactor (16000000) // Vdd=1.8-2.1
//#  define FlashReadLatencyFactor (18000000) // Vdd=2.1-2.4
//#  define FlashReadLatencyFactor (24000000) // Vdd=2.4-2.7
#  define FlashReadLatencyFactor (30000000) // Vdd=2.7-


#ifdef SysClockPLL_USE
#	if ( (SysClockPLL_N > 432) || (SysClockPLL_N < 64) )
#		error "out of range PLL_N(64..432)."
#	endif
#	if ( (SysClockPLL_M > 63) || (SysClockPLL_M < 2) )
#		error "out of range PLL_M(2..63)."
#	endif
#	if ( (SysClockPLL_Q > 15) || (SysClockPLL_Q < 2) )
#		error "out of range PLL_Q(2..15)."
#	endif
#	if ( (SysClockPLL_P != 2) && (SysClockPLL_P != 4) && (SysClockPLL_P != 6) && (SysClockPLL_P != 8) )
#		error "out of range PLL_P(2,4,6,8)."
#	endif
#	define	calcRCC_PLLCFG  ( (SysClockPLL_Q << 24) | (((SysClockPLL_P / 2) - 1)<<16) | (SysClockPLL_N << 6) | SysClockPLL_M )

#  define AHBClock (((SysClock * SysClockPLL_N) / SysClockPLL_M) / SysClockPLL_P)
#  if (AHBClock) > 168000000
#		error "over clock assumed! (AHBclock>168MHz)"
#  endif
#  if (AHBClock) > 144000000
#		warning "confirm set the VOS_bit in PWR_CR."
#  endif

#  define FlashReadLatency (AHBClock/FlashReadLatencyFactor)
#  if ( FlashReadLatency > 7 )
#		error "out of Flash Memry READ Latency cycles."
#  endif

#  if (AHBClock) > 84000000
#    define	APB1div   (4)
#    define	APB2div   (2)
#    define	CFGR_PPRE1 (5)
#    define	CFGR_PPRE2 (4)
#  else
#    if (AHBClock) > 42000000
#      define	APB1div   (2)
#      define	APB2div   (1)
#      define	CFGR_PPRE1 (4)
#      define	CFGR_PPRE2 (0)
#    else
#      define	APB2div   (1)
#      define	APB1div   (1)
#      define	CFGR_PPRE1 (0)
#      define	CFGR_PPRE2 (0)
#    endif
#  endif
#else
#  define	AHBClock  (SysClock)
#  define	APB1div   (1)
#  define	APB2div   (1)
#  define	CFGR_PPRE1 (0)
#  define	CFGR_PPRE2 (0)
#endif

#define	CFGR_HPRE  (0)
#define	CPUClock  (AHBClock)
#define	APB1Clock (AHBClock / APB1div)
#define	APB2Clock (AHBClock / APB2div)

#define	DI	{__disable_irq();}
#define	EI	{__enable_irq();}
#define loop	while(1)

#endif /* SYSCLK_CONFIG_H */
