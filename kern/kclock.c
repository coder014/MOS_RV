#include <kclock.h>
#include <sbi.h>
#include <riscv.h>

static inline unsigned long long get_time(void)
{
    u_long lo, hi, tmp;
    asm volatile(
        "1:\n"
        "rdtimeh %0\n"
        "rdtime %1\n"
        "rdtimeh %2\n"
        "bne %0, %2, 1b"
        : "=&r"(hi), "=&r"(lo), "=&r"(tmp));
    return ((unsigned long long)hi << 32) | lo;
}

static inline void sbi_set_timer(unsigned long long stime_value) {
    sbi_call(SBI_SET_TIMER, (u_int)stime_value, (u_int)(stime_value >> 32), 0);
}

void kclock_init(void) {
    set_csr(sie, MIP_STIP);
    clock_set_next_event();
}

void clock_set_next_event()
{
	sbi_set_timer(get_time() + KLOCK_TIMEBASE);
}