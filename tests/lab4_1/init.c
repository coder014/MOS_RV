void rv32_init(u_int hartid, u_int dtb) {
	printk("init.c:\trv32_init() is called\n");
	riscv_detect_memory(dtb);
	riscv_vm_init();
	page_init();
	env_init();

	envid2env_check();
	halt();
}
