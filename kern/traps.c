#include <trap.h>
#include <riscv.h>
#include <printk.h>
#include <kclock.h>
#include <pmap.h>
#include <mmu.h>
#include <sched.h>

void idt_init()
{
	extern void exc_gen_entry(void);
	asm volatile("csrw sscratch, x0");
	write_csr(stvec, &exc_gen_entry);
	set_csr(sstatus, SSTATUS_SIE);
}

static void interrupt_handler(struct Trapframe *tf)
{
	u_int cause = tf->cause & 0x7FFFFFFFU;
	switch (cause) {
		case IRQ_S_TIMER:
			clock_set_next_event();
			if (!cur_pgdir) {
				//initial timer interrupt, clear SSTATUS_SPP !!!
				clear_csr(sstatus, SSTATUS_SPP);
			}
			schedule(0);
			break;
		default:
			panic("unhandled intr: %d\n", cause);
	}
}

static void normal_page_fault(struct Trapframe *tf)
{
	u_int tmp = tf->status;
	assert(!(tmp & SSTATUS_SPP));
	tmp = read_csr(satp);
	_do_tlb_refill(tf->badvaddr, (tmp & 0x7FFFFFFF) >> 22);
}

static void exception_handler(struct Trapframe *tf)
{
	switch (tf->cause) {
		case CAUSE_FAULT_STORE:
			normal_page_fault(tf);
			break;
		default:
			panic("unhandled exception: %d\n", tf->cause);
	}
}

void traps_dispatch(struct Trapframe *tf)
{
	if (tf->cause >= 0x80000000U) {
		interrupt_handler(tf);
	} else {
		exception_handler(tf);
	}
}