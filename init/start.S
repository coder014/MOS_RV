#include <asm/asm.h>
#include <mmu.h>

	.section .boot,"ax",%progbits
EXPORT(_start)
	/* hint: you can reference the memory layout in include/mmu.h */
	/* set up the kernel stack */
	li sp, KSTACKTOP

	/* jump to _init */
	j rv32_init
