/*
 * NanoOS application linker script
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
		
		_init_glob_obj_start = .;
		KEEP(*(.text._GLOBAL__sub_I_*))
		_init_glob_obj_end = .;
		
		*(.text .text.*)
		*(.rodata .rodata.*)
	} > APP_FLASH
	
	/*
	.rodata :
	{
		*(.rodata .rodata.*)
	} > APP_FLASH
	*/
	
	.ARM.extab :
	{
		*(.ARM.extab* .gnu.linkonce.armextab.*)
	} > APP_FLASH
	
	.ARM.exidx :
	{
		*(.ARM.exidx* .gnu.linkonce.armexidx.*)
	} > APP_FLASH
	
	_etext = .;
	
	.data : AT(_etext)
	{
		_start_data = .;
		*(.data .data.*)
		. = ALIGN(4);
		_end_data = .;
	} > APP_SRAM
	
	.bss (NOLOAD) :
	{
		_start_bss = .;
		*(.bss .bss.*)
		. = ALIGN(4);
		_end_bss = .;
	} > APP_SRAM
}
