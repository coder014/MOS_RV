#include <trap.h>
#include <env.h>
#include <riscv.h>
#include <printk.h>
#include <kclock.h>
#include <pmap.h>
#include <mmu.h>
#include <sched.h>
#include <syscall.h>

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
	// assert(!(tmp & SSTATUS_SPP));
	tmp = read_csr(satp);
	_do_tlb_refill(tf->badvaddr, (tmp & 0x7FFFFFFF) >> 22);
}

static void query_vpt(struct Trapframe *tf)
{
	u_int instr = *(u_int *)tf->epc;
	if ((instr & 0b1111111) != 0b0000011) panic("user querying vpt not using LW");
	if (((instr >> 12) & 0b111) != 0b010) panic("user querying vpt not using LW");
	if (!cur_pgdir) panic("cur_pgdir is NULL");
	u_int va = tf->badvaddr << 10;
	Pte *res;
	if (va >= UVPT && va < ULIM) {
		res = cur_pgdir + PTX(va);
	} else if (va < UVPT) {
		pgdir_walk(cur_pgdir, va, 0, &res);
	} else {
		panic("trying to fetch kernel space vpt\n");
	}
	tf->regs[(instr >> 7) & 0b11111] = res ? *res : 0;
	tf->epc += 4;
}

static void user_mod(struct Trapframe *tf) {
	struct Trapframe tmp_tf = *tf;

	if (tf->regs[2] < USTACKTOP || tf->regs[2] >= UXSTACKTOP) {
		tf->regs[2] = UXSTACKTOP;
	}
	tf->regs[2] -= sizeof(struct Trapframe);
	*(struct Trapframe *)tf->regs[2] = tmp_tf;

	if (curenv->env_user_tlb_mod_entry) {
		tf->regs[10] = tf->regs[2];
		// Hint: Set 'cp0_epc' in the context 'tf' to 'curenv->env_user_tlb_mod_entry'.
		/* Exercise 4.11: Your code here. */
		tf->epc = curenv->env_user_tlb_mod_entry;
	} else {
		panic("TLB Mod but no user handler registered");
	}
}


static void exception_handler(struct Trapframe *tf)
{
	Pte *tmp;
	switch (tf->cause) {
		case CAUSE_FAULT_FETCH:
			panic("unknown instr at %08x : %08x", tf->epc, *(u_int*)tf->epc);
			break;
		case CAUSE_USER_ECALL:
			do_syscall(tf);
			break;
		case CAUSE_FAULT_LOAD:
			if (tf->badvaddr >= UVPT && tf->badvaddr < ULIM) query_vpt(tf);
			else normal_page_fault(tf);
			break;
		case CAUSE_FAULT_STORE:
			pgdir_walk(cur_pgdir, tf->badvaddr, 0, &tmp);
			if (tmp && (*tmp & PTE_V) && !(*tmp & PTE_W)) user_mod(tf);
			else normal_page_fault(tf);
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