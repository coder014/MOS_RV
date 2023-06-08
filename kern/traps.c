#include <trap.h>
#include <riscv.h>
#include <printk.h>
#include <kclock.h>
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
			//printk("timer intr!\n");
			schedule(0);
			break;
		default:
			panic("unhandled intr: %d\n", cause);
	}
}

static void exception_handler(struct Trapframe *tf)
{
	switch (tf->cause) {
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