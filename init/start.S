#include <asm/asm.h>
#include <mmu.h>

    .section .text,"ax",%progbits
    .globl _start
	.type _start, @function
_start:
	/* hint: you can reference the memory layout in include/mmu.h */
	/* set up the kernel stack */
	li sp, KSTACKTOP

	/* jump to _init */
	j rv32_init
