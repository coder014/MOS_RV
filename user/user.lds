OUTPUT_ARCH(riscv)

/*
 * Set the ENTRY point of the program to _start.
 */
ENTRY(_start)

PHDRS {
	code PT_LOAD FLAGS (5);
	data PT_LOAD FLAGS (6);
}

SECTIONS {
	. = 0x00400000;

	.text : {
		*(.text .text.*)
		*(.rodata .rodata.*)
	} : code

	.data ALIGN(4096) : {
		*(.data .data.* .sdata .sdata.*)
	} : data

	.bss ALIGN(4096) : {
		*(.bss .bss.* .sbss .sbss.*)
	} : data

	end = . ;
}
