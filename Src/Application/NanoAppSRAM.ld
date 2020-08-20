/*
 * Run in SRAM linker script
 */

MEMORY
{
	APP_FLASH	(rx)	: ORIGIN = 0x00008000, LENGTH = 31K
	CFG_FLASH	(rw)	: ORIGIN = 0x0000FC00, LENGTH = 1K
	APP_SRAM	(rwx)	: ORIGIN = 0x20000000, LENGTH = 10K
}

SECTIONS
{
	.text :
	{
		KEEP(*(.nanoos_application))
		
		__start_global_object_init = .;
		KEEP(*(.text._GLOBAL__sub_I_*))
		__end_global_object_init = .;
		
		*(.text .text.*)
		*(.rodata .rodata.*)
	} > APP_SRAM
	
	_etext = .;
	
	.data : AT(_etext)
	{
		_start_data = .;
		*(.data .data.*)
		. += 4;
		_end_data = .;
	} > APP_SRAM
	
	.bss (NOLOAD) :
	{
		_start_bss = .;
		*(.bss .bss.*)
		. += 4;
		_end_bss = .;
	} > APP_SRAM
}
