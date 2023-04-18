#ifndef __ASM_RISCV_REGDEF_H
#define __ASM_RISCV_REGDEF_H

/*
 * Symbolic register names for 32 bit ABI
 */
#define zero x0 /* wired zero */
#define ra x1 /* return address */
#define sp x2 /* stack pointer */
#define gp x3 /* global pointer */
#define tp x4 /* thread pointer */
#define t0 x5 /* caller saved */ /* alternate link register */
#define t1 x6
#define t2 x7
#define s0 x8 /* callee saved */
#define fp x8 /* frame pointer */
#define s1 x9
#define a0 x10 /* argument registers */	/* return value */
#define a1 x11	/* return value */
#define a2 x12
#define a3 x13
#define a4 x14
#define a5 x15
#define a6 x16
#define a7 x17
#define s2 x18 /* callee saved */
#define s3 x19
#define s4 x20
#define s5 x21
#define s6 x22
#define s7 x23
#define s8 x24
#define s9 x25
#define s10 x26
#define s11 x27
#define t3 x28 /* caller saved */
#define t4 x29
#define t5 x30
#define t6 x31

#endif /* __ASM_RISCV_REGDEF_H */
