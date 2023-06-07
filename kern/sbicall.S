#include <asm/asm.h>

FEXPORT(sbi_call)
	// addi sp, sp, -16
	// sw a7, 0(sp)
	mv a7, a0 // set a7 to EID
	mv a0, a1 // set a0 to arg0
	mv a1, a2
	mv a2, a3
	ecall
	// lw a7, 0(sp)
	// addi sp, sp, 16
	ret
END(sbi_call)