MEMORY {
	rom(xwr)	: ORIGIN = 0x08000000, LENGTH = 1024k	/* internal	flash rom */
/*	ram(xwr)	: ORIGIN = 0x10000000, LENGTH =   60k	/* internal	ram	(CCM: MPU access only) */
/*	stack(xwr)	: ORIGIN = 0x1000f000, LENGTH =    4k	/* internal	ram	(CCM: MPU access only) */
	stack(xwr)	: ORIGIN = 0x10000000, LENGTH =   64k	/* internal	ram	(CCM: MPU access only) */
	ram(xwr)	: ORIGIN = 0x20000000, LENGTH =  128k	/* internal	ram	(able to use w/DMA) */
	nvram(wr)	: ORIGIN = 0x40024000, LENGTH =    4k	/* battery backup memory */
}

_startof_flash = ORIGIN(rom);            /* Start of available flash memory address */
_endof_flash = ORIGIN(rom) + LENGTH(rom);   /* End of available above address + 1 */
_startof_sram = ORIGIN(ram);             /* Start of available SRAM address */
_endof_sram = ORIGIN(ram) + LENGTH(ram);    /* End of available above address + 1 */
_startof_nvram = ORIGIN(nvram);          /* Start of available battery backup SRAM address */
_endof_nvram = ORIGIN(nvram)+LENGTH(nvram); /* End of available above address + 1 */
_startof_stack = ORIGIN(stack);          /* Start of available inner-SRAM address */
_endof_stack = ORIGIN(stack)+LENGTH(stack); /* End of available above address + 1 */

SECTIONS {
	.text : {	FILL(0xff)
		KEEP(*(.VECTOR))       /* Exception vector table */
		. =	ALIGN(4);
		_text =	.;
		PROVIDE(stext =	.);
		KEEP(*(.isr_vector))
		KEEP(*(.init))
		*(.text	.text.*)		
		_fini =	. ;
		*(.fini)
		*(.rodata .rodata.*)		
		*(.gnu.linkonce.t.*)
		*(.glue_7)
		*(.glue_7t)
		*(.gcc_except_table)
		*(.gnu.linkonce.r.*)
		. =	ALIGN(4);
	} >rom

	.ARM.excep : {	/* Data table for library exception handling */
		*(.ARM.exidx*)
		*(.ARM.extab*)
	} >rom

	. = ALIGN(4);
	_etext = .;         /* End of .text section */
	PROVIDE(etext =	.);	  
	_sidata = .;        /* Start of the initialization values of the .data section */

	.data : AT (_sidata) {	FILL(0xff)
		. =	ALIGN(4);
		_sdata = .;
		*(.ramfunc .ramfunc.* .fastrun .fastrun.*)
		*(.data	.data.*)
		*(.gnu.linkonce.d.*)
		. =	ALIGN(4);
		_edata = .;
	} >ram

	.bss (NOLOAD) :	{
		. =	ALIGN(4);
		/* This	is used	by the startup in order	to initialize the .bss secion */
		_sbss =	.;
		*(.bss .bss.*)
		*(.gnu.linkonce.b.*)
		*(COMMON)
		. =	ALIGN(4);		 
		_ebss =	.;
		_end  = .;
		PROVIDE(_end =	.);	  
		PROVIDE(end =	.);	  
		PROVIDE(_heap =	.);	  
	} >ram

	.nonvolatile (NOLOAD) : ALIGN(4) {
		_snonvolatile = .;
		*(.nonvolatile .nonvolatile.*)
		_enonvolatile = .;
	} >nvram

	.stack (NOLOAD) : ALIGN(16) {
		*(.stack)
		*(.STACK)
		. = ALIGN(4);
	} >stack
	. = ALIGN(16);
}

