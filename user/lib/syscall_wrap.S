#include <asm/asm.h>

FEXPORT(msyscall)
	// Just use 'syscall' instruction and return.

	/* Exercise 4.1: Your code here. */
	ecall
	jr ra

END(msyscall)
