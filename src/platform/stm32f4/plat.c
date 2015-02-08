/** @file
 * PyMite - A flyweight Python interpreter for 8-bit and larger microcontrollers.
 * Copyright 2002 Dean Hall.  All rights reserved.
 * PyMite is offered through one of two licenses: commercial or open-source.
 * See the LICENSE file at the root of this package for licensing details.
 *
 * some sections based on code (C) COPYRIGHT 2008 STMicroelectronics
 */

#undef __FILE_ID__
#define __FILE_ID__ 0x74

#include <stdio.h>

#include "stm32f4xx.h"
#include "sysclk_config.h"
#include "core_cm4.h"

#include "pm.h"

extern volatile uint32_t CNT_tick;
extern volatile uint32_t fCNT_tick;
extern volatile uint32_t dCNT_tick[];

extern int16_t USART2_getc(void);
extern void USART2_putc(char);
extern void USART2_puts(char *);

#define putstr	USART2_puts
#define putch	USART2_putc
#define getch	USART2_getc

/*
 * Retargets the C library printf function to the USART.
 */
int fputc(int ch, FILE *f)
{
    plat_putByte((uint8_t) ch);
    return ch;
}


PmReturn_t
plat_init(void)
{
    return PM_RET_OK;
}


PmReturn_t
plat_deinit(void)
{
    return PM_RET_OK;
}


PmReturn_t
plat_getMsTicks(uint32_t *r_ticks)
{
    *r_ticks = CNT_tick;
//    *r_ticks = pm_timerMsTicks;

    return PM_RET_OK;
}


/*
 * Gets a byte from the address in the designated memory space
 * Post-increments *paddr.
 */
uint8_t
plat_memGetByte(PmMemSpace_t memspace, uint8_t const **paddr)
{
    uint8_t b = 0;

    switch (memspace)
    {
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


PmReturn_t
plat_getByte(uint8_t *b)
{
    int c;
    PmReturn_t retval = PM_RET_OK;

	c= getch();

    *b = c & 0xFF;

    if (c > 0xFF)
    {
        PM_RAISE(retval, PM_RET_EX_IO);
    }

    return retval;
}


PmReturn_t
plat_putByte(uint8_t b)
{
	putch(b);
    return PM_RET_OK;
}


void
plat_reportError(PmReturn_t result)
{
     /* Print error */
	switch(result){
	  case(PM_RET_EX):
		printf("PM_RET_EX: General exception\n");
		break;;
	  case(PM_RET_EX_EXIT):
		printf("PM_RET_EX_EXIT: System exit\n");
		break;;
	  case(PM_RET_EX_IO):
		printf("PM_RET_EX_IO: Input/output error\n");
		break;;
	  case(PM_RET_EX_ZDIV):
		printf("PM_RET_EX_ZDIV: Zero division error\n");
		break;;
	  case(PM_RET_EX_ASSRT):
		printf("PM_RET_EX_ASSRT: Assertion error\n");
		break;;
	  case(PM_RET_EX_ATTR):
		printf("PM_RET_EX_ATTR: Attribute error\n");
		break;;
	  case(PM_RET_EX_IMPRT):
		printf("PM_RET_EX_IMPRT: Import error\n");
		break;;
	  case(PM_RET_EX_INDX):
		printf("PM_RET_EX_INDX: Index error\n");
		break;;
	  case(PM_RET_EX_KEY):
		printf("PM_RET_EX_KEY: Key error\n");
		break;;
	  case(PM_RET_EX_MEM):
		printf("PM_RET_EX_MEM: Memory error\n");
		break;;
	  case(PM_RET_EX_NAME):
		printf("PM_RET_EX_NAME: Name error\n");
		break;;
	  case(PM_RET_EX_SYNTAX):
		printf("PM_RET_EX_SYNTAX: Syntax error\n");
		break;;
	  case(PM_RET_EX_SYS):
		printf("PM_RET_EX_SYS: System error\n");
		break;;
	  case(PM_RET_EX_TYPE):
		printf("PM_RET_EX_TYPE: Type error\n");
		break;;
	  case(PM_RET_EX_VAL):
		printf("PM_RET_EX_VAL: Value error\n");
		break;;
	  case(PM_RET_EX_STOP):
		printf("PM_RET_EX_STOP: Stop iteration\n");
		break;;
	  case(PM_RET_EX_WARN):
		printf("PM_RET_EX_WARN: Warning\n");
		break;;
	  default:
		printf("???: unknown result value.\n");
		break;;
	};
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
        pframe = (pPmObj_t)gVmGlobal.pthread->pframe;

        /* If it's the native frame, print the native function name */
        if (pframe == (pPmObj_t)&(gVmGlobal.nativeframe))
        {

            /* The last name in the names tuple of the code obj is the name */
            retval = tuple_getItem((pPmObj_t)gVmGlobal.nativeframe.nf_func->
                                   f_co->co_names, -1, &pstr);
            if ((retval) != PM_RET_OK)
            {
                printf("  Unable to get native func name.\n");
                return;
            }
            else
            {
                printf("  %s() __NATIVE__\n", ((pPmString_t)pstr)->val);
            }

            /* Get the frame that called the native frame */
            pframe = (pPmObj_t)gVmGlobal.nativeframe.nf_back;
        }

        /* Print the remaining frame stack */
        for (;
             pframe != C_NULL;
             pframe = (pPmObj_t)((pPmFrame_t)pframe)->fo_back)
        {
            /* The last name in the names tuple of the code obj is the name */
            retval = tuple_getItem((pPmObj_t)((pPmFrame_t)pframe)->
                                   fo_func->f_co->co_names, -1, &pstr);
            if ((retval) != PM_RET_OK) break;

            printf("  %s()\n", ((pPmString_t)pstr)->val);
        }
        printf("  <module>.\n");
    }
}
