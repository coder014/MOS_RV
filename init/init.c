#include <types.h>
#include <printk.h>
#include <mmu.h>
#include <console.h>
#include <pmap.h>
#include <trap.h>
#include <kclock.h>
#include <env.h>

// When build with 'make test lab=?_?', we will replace your 'mips_init' with a generated one from
// 'tests/lab?_?'.
#ifdef MOS_INIT_OVERRIDDEN
#include <generated/init_override.h>
#else

void rv32_init(u_int hartid, u_int dtb) { // two args from previous hart
	printk("init.c:\trv32_init() is called\n");

	// lab2:
	riscv_detect_memory(dtb);
	riscv_vm_init();
	page_init();

	// lab3:
	env_init();

	// lab3:
	// ENV_CREATE_PRIORITY(user_bare_loop, 1);
	// ENV_CREATE_PRIORITY(user_bare_loop, 2);

	// lab4:
	// ENV_CREATE(user_tltest);
	// ENV_CREATE(user_fktest);
	// ENV_CREATE(user_pingpong);

	// lab6:
	ENV_CREATE(user_icode);  // This must be the first env!

	// lab5:
	// ENV_CREATE(user_fstest);
	ENV_CREATE(fs_serv);  // This must be the second env!
	// ENV_CREATE(user_devtst);

	// lab3:
	kclock_init();
	idt_init();
	while (1) {
	}
	panic("init.c:\tend of rv32_init() reached!");
}

#endif
