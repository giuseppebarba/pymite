// 
// STM32F4discoveryボードでpymiteを走らせる メインプログラム
// 参考資料：
//	RM0090 Reference manual - STM32F4xx advanced ARM-based 32-bit MCUs
//	PM0056 Programming manual - STM32Fxxx Cortex-M3 programming manual
//	UM1472 Users manual - STM32F4DISCOVERY discovery board
// 
#include "stm32f4xx.h"
#include "core_cm4.h"
#include "sysclk_config.h"

#include "pm.h"
#define HEAP_SIZE 0x7000


extern unsigned char usrlib_img[];

int main(void){
    uint8_t heap[HEAP_SIZE];
    PmReturn_t retval;

    /* Init PyMite */
    retval = pm_init(heap, HEAP_SIZE, MEMSPACE_PROG, usrlib_img);

    PM_RETURN_IF_ERROR(retval);

    /* Run the sample program */
    retval = pm_run((uint8_t *)"main");

    return (int)retval;
}


