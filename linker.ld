SECTIONS {
	. = 0x8000;
	.text : { *(.text .text.*) }
	.rodata ALIGN(4) : { *(.rodata .rodata.*) }
	.data ALIGN(4) : { *(.data .data.*) }
	. = 0x600000;
	.bss : { *(.bss .bss.*) *(COMMON) }
	. = ALIGN(8);
	_end = .;
}
