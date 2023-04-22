#include <asm/asm.h>
#include <mmu.h>
#include <trap.h>

.macro SAVE_ALL
	csrw    sscratch, sp
	bltz    sp, 1f
	li      sp, KSTACKTOP
1:
	subi    sp, sp, TF_SIZE
	sw      x0, TF_REG0(sp)
	sw      x1, TF_REG1(sp)
	sw      x3, TF_REG3(sp)
	sw      x4, TF_REG4(sp)
	sw      x5, TF_REG5(sp)
	sw      x6, TF_REG6(sp)
	sw      x7, TF_REG7(sp)
	sw      x8, TF_REG8(sp)
	sw      x9, TF_REG9(sp)
	sw      x10, TF_REG10(sp)
	sw      x11, TF_REG11(sp)
	sw      x12, TF_REG12(sp)
	sw      x13, TF_REG13(sp)
	sw      x14, TF_REG14(sp)
	sw      x15, TF_REG15(sp)
	sw      x16, TF_REG16(sp)
	sw      x17, TF_REG17(sp)
	sw      x18, TF_REG18(sp)
	sw      x19, TF_REG19(sp)
	sw      x20, TF_REG20(sp)
	sw      x21, TF_REG21(sp)
	sw      x22, TF_REG22(sp)
	sw      x23, TF_REG23(sp)
	sw      x24, TF_REG24(sp)
	sw      x25, TF_REG25(sp)
	sw      x26, TF_REG26(sp)
	sw      x27, TF_REG27(sp)
	sw      x28, TF_REG28(sp)
	sw      x29, TF_REG29(sp)
	sw      x30, TF_REG30(sp)
	sw      x31, TF_REG31(sp)
	csrr    t0, sscratch
	sw      t0, TF_REG2(sp)
	csrr    t0, sstatus
	sw      t0, TF_STATUS(sp)
	csrr    t0, scause
	sw      t0, TF_CAUSE(sp)
	csrr    t0, sepc
	sw      t0, TF_EPC(sp)
	csrr    t0, sbadaddr
	sw      t0, TF_BADVADDR(sp)
.endm

.macro RESTORE_SOME
	lw      t0, TF_STATUS(sp)
	lw      t1, TF_EPC(sp)
	csrw    sstatus, t0
	csrw    sepc, t1
	lw      x31, TF_REG31(sp)
	lw      x30, TF_REG30(sp)
	lw      x29, TF_REG29(sp)
	lw      x28, TF_REG28(sp)
	lw      x27, TF_REG27(sp)
	lw      x26, TF_REG26(sp)
	lw      x25, TF_REG25(sp)
	lw      x24, TF_REG24(sp)
	lw      x23, TF_REG23(sp)
	lw      x22, TF_REG22(sp)
	lw      x21, TF_REG21(sp)
	lw      x20, TF_REG20(sp)
	lw      x19, TF_REG19(sp)
	lw      x18, TF_REG18(sp)
	lw      x17, TF_REG17(sp)
	lw      x16, TF_REG16(sp)
	lw      x15, TF_REG15(sp)
	lw      x14, TF_REG14(sp)
	lw      x13, TF_REG13(sp)
	lw      x12, TF_REG12(sp)
	lw      x11, TF_REG11(sp)
	lw      x10, TF_REG10(sp)
	lw      x9, TF_REG9(sp)
	lw      x8, TF_REG8(sp)
	lw      x7, TF_REG7(sp)
	lw      x6, TF_REG6(sp)
	lw      x5, TF_REG5(sp)
	lw      x4, TF_REG4(sp)
	lw      x3, TF_REG3(sp)
	lw      x1, TF_REG1(sp)
	lw      x2, TF_REG2(sp)
.endm
