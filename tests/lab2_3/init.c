extern u_long npage;
extern int pgdir_walk(Pde *pgdir, u_long va, int create, Pte **ppte); // set the function to non-static

void mmu_check(u_int *data) {
	u_int ldata[] = {3473356703, 3343942787, 29, 11, 7, 5, 3, 2};
	struct Page *pp;
	assert(page_alloc(&pp) == 0);
	Pde *boot_pgdir = page2pa(pp);
	for(u_int i = KERNBASE; i < KSEG0 + npage * BY2PG; i += BY2PG) {
		Pte *pte;
		assert(pgdir_walk(boot_pgdir, i, 1, &pte) == 0);
		*pte = PADDR2PTE(i) | PTE_A | PTE_D | PTE_R | PTE_W | PTE_X | PTE_V;
	}

	// switch to virtual address mode
	u_int atp = 0x80000000U | PPN(boot_pgdir);
	printk("before entering virtual addressing...\n");
	asm volatile("csrw satp, %0\nnop" :: "r"(atp) : "memory");
	printk("entered virtual addressing!\n");

	for(int i = 0; i < 8; i++) {
		assert(ldata[i] == data[8-i-1]);
	}
	// directly read kernel entry's code
	// 0x80600137 == lui sp, 0x80600
	assert(*(u_int*)(KERNBASE) == 0x80600137U);
	printk("test point 1 ok\n");

	data[6] = 666; data[7] = 777;
	// switch to physical address mode
	printk("before entering physical addressing...\n");
	asm volatile("csrw satp, x0\nnop" ::: "memory");
	printk("entered physical addressing!\n");
}

void rv32_init(u_int hartid, u_int dtb) {
	printk("init.c:\trv32_init() is called\n");

	riscv_detect_memory(dtb);
	riscv_vm_init();
	page_init();

	u_int data[] = {2, 3, 5, 7, 11, 29, 3343942787, 3473356703};

	mmu_check(data);
	assert(data[6]==666);
	assert(data[7]==777);
	printk("test point 2 ok\n");
	halt();
}
