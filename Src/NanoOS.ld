MEMORY
{
	SYS_FLASH		(rx)	: ORIGIN = 0x00000000, LENGTH = 32K
	APP_FLASH		(rx)	: ORIGIN = 0x00008000, LENGTH = 31K
	CFG_FLASH		(rw)	: ORIGIN = 0x0000FC00, LENGTH = 1K
	APP_SRAM		(rwx)	: ORIGIN = 0x20000000, LENGTH = 10K
	SYS_SRAM		(rw)	: ORIGIN = 0x20002800, LENGTH = 10K
}

SECTIONS
{
	.text :
	{
		KEEP(*(.nanoos_isr))
		*(.text .text.*)
	} > SYS_FLASH
	
	.rodata :
	{
		*(.rodata .rodata.*)
	} > SYS_FLASH
	
	_etext = .;
	
	
	.nanoos_application :
	{
		KEEP(*(.nanoos_application))
		. = ALIGN(31744);
	} > APP_FLASH
	
	.nanoos_config :
	{
		KEEP(*(.nanoos_config))
		. = ALIGN(1024);
	} > CFG_FLASH
	
	
	.data : AT(_etext)
	{
		_start_data = .;
		*(.data .data.*)
		. += 4;
		_end_data = .;
	} > SYS_SRAM
	
	.bss (NOLOAD) :
	{
		_start_bss = .;
		*(.bss .bss.*)
		. += 4;
		_end_bss = .;
	} > SYS_SRAM
}