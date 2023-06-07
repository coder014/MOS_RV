#ifndef _KCLOCK_H_
#define _KCLOCK_H_
#ifndef __ASSEMBLER__
#define KLOCK_TIMEBASE 50000 // 5ms = 200Hz
void kclock_init(void);
void clock_set_next_event(void);
#endif /* !__ASSEMBLER__ */
#endif
