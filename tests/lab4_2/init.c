void rv32_init(u_int hartid, u_int dtb) {
	printk("init.c:\trv32_init() is called\n");
	riscv_detect_memory(dtb);
	riscv_vm_init();
	page_init();
	env_init();

	struct Env *ppb = ENV_CREATE_PRIORITY(test_ppb, 5);
	struct Env *ppc = ENV_CREATE_PRIORITY(test_ppc, 5);
	ppc->env_parent_id = ppb->env_id;

	kclock_init();
	idt_init();
	while (1) {
	}
	panic("init.c:\tend of rv32_init() reached!");
}
