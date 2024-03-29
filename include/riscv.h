#ifndef __LIBS_RISCV_H__
#define __LIBS_RISCV_H__

#define IRQ_U_SOFT   0
#define IRQ_S_SOFT   1
#define IRQ_H_SOFT   2
#define IRQ_M_SOFT   3
#define IRQ_U_TIMER  4
#define IRQ_S_TIMER  5
#define IRQ_H_TIMER  6
#define IRQ_M_TIMER  7
#define IRQ_U_EXT    8
#define IRQ_S_EXT    9
#define IRQ_H_EXT    10
#define IRQ_M_EXT    11
#define IRQ_COP      12
#define IRQ_HOST     13

#define CAUSE_MISALIGNED_FETCH 0x0
#define CAUSE_ACCESS_FETCH 0x1
#define CAUSE_ILLEGAL_INSTRUCTION 0x2
#define CAUSE_BREAKPOINT 0x3
#define CAUSE_MISALIGNED_LOAD 0x4
#define CAUSE_ACCESS_LOAD 0x5
#define CAUSE_MISALIGNED_STORE 0x6
#define CAUSE_ACCESS_STORE 0x7
#define CAUSE_USER_ECALL 0x8
#define CAUSE_SUPERVISOR_ECALL 0x9
#define CAUSE_FAULT_FETCH 0xc
#define CAUSE_FAULT_LOAD 0xd
#define CAUSE_FAULT_STORE 0xf

#define MIP_SSIP            (1 << IRQ_S_SOFT)
#define MIP_STIP            (1 << IRQ_S_TIMER)
#define MIP_SEIP            (1 << IRQ_S_EXT)

#define SIP_SSIP MIP_SSIP
#define SIP_STIP MIP_STIP
#define SIP_SEIP MIP_SEIP

#define SSTATUS_SIE         0x00000002
#define SSTATUS_SPIE        0x00000020
#define SSTATUS_SPP         0x00000100
#define SSTATUS_SUM         0x00040000

#ifndef __ASSEMBLER__

#define read_csr(reg) ({ unsigned long __tmp; \
  asm volatile ("csrr %0, " #reg : "=r"(__tmp)); \
  __tmp; })

#define write_csr(reg, val) ({ \
    asm volatile ("csrw " #reg ", %0" :: "r"(val)); })

#define swap_csr(reg, val) ({ unsigned long __tmp; \
    asm volatile ("csrrw %0, " #reg ", %1" : "=r"(__tmp) : "r"(val)); \
  __tmp; })

#define set_csr(reg, bit) ({ unsigned long __tmp; \
    asm volatile ("csrrs %0, " #reg ", %1" : "=r"(__tmp) : "r"(bit)); \
  __tmp; })

#define clear_csr(reg, bit) ({ unsigned long __tmp; \
    asm volatile ("csrrc %0, " #reg ", %1" : "=r"(__tmp) : "r"(bit)); \
  __tmp; })

#endif
#endif