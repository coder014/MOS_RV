#include <pmap.h>
#include <printk.h>
#include <fdt.h>
#include <riscv.h>

/* These variables are set by mips_detect_memory() */
static u_long memsize; /* Maximum physical address */
u_long npage;	       /* Amount of memory(in pages) */
Pde *kbasepgdir;
Pde *cur_pgdir;

struct Page *pages;
static u_long freemem;

struct Page_list page_free_list; /* Free list of physical pages */

/* Overview:
 *   Read memory size from DEV_MP to initialize 'memsize' and calculate the corresponding 'npage'
 *   value.
 */
void riscv_detect_memory(u_int dtb) {
	/* Step 1: Initialize memsize. */
	memsize = query_mem_size(dtb);

	/* Step 2: Calculate the corresponding 'npage' value. */
	/* Exercise 2.1: Your code here. */
	npage = memsize / BY2PG;

	printk("Memory size: %lu KiB, number of pages: %lu\n", memsize / 1024, npage);
}

/* Overview:
    Allocate `n` bytes physical memory with alignment `align`, if `clear` is set, clear the
    allocated memory.
    This allocator is used only while setting up virtual memory system.
   Post-Condition:
    If we're out of memory, should panic, else return this address of memory we have allocated.*/
void *alloc(u_int n, u_int align, int clear) {
	extern char end[];
	u_long alloced_mem;

	/* Initialize `freemem` if this is the first time. The first virtual address that the
	 * linker did *not* assign to any kernel code or global variables. */
	if (freemem == 0) {
		freemem = (u_long)end; // end
	}

	/* Step 1: Round up `freemem` up to be aligned properly */
	freemem = ROUND(freemem, align);

	/* Step 2: Save current value of `freemem` as allocated chunk. */
	alloced_mem = freemem;

	/* Step 3: Increase `freemem` to record allocation. */
	freemem = freemem + n;

	// Panic if we're out of memory.
	panic_on(freemem - KSEG0 >= memsize);

	/* Step 4: Clear allocated chunk if parameter `clear` is set. */
	if (clear) {
		memset((void *)alloced_mem, 0, n);
	}

	/* Step 5: return allocated chunk. */
	return (void *)alloced_mem;
}

/* Overview:
    Set up two-level page table.
   Hint:
    You can get more details about `UPAGES` and `UENVS` in include/mmu.h. */
void riscv_vm_init() {
	/* Allocate proper size of physical memory for global array `pages`,
	 * for physical memory management. Then, map virtual address `UPAGES` to
	 * physical address `pages` allocated before. For consideration of alignment,
	 * you should round up the memory size before map. */
	pages = (struct Page *)alloc(npage * sizeof(struct Page), BY2PG, 1);
	printk("to memory %x for struct Pages and kernelbase pagedir.\n", freemem);
	kbasepgdir = (u_long *)alloc(BY2PG, BY2PG, 1);
	for(u_long i = KSEG0; i < KSEG0 + memsize; i+=PDMAP) {
		kbasepgdir[PDX(i)] = PADDR2PTE(i) | PTE_G | PTE_A | PTE_D | PTE_R | PTE_W | PTE_X | PTE_V;
		//printk("kbasepgdir[%d] = %08lx\n", PDX(i), kbasepgdir[PDX(i)]);
	}
	kbasepgdir[PDX(KMMIO)] = 0x04000000U | PTE_G | PTE_A | PTE_D | PTE_R | PTE_W | PTE_V;
	u_int atp = 0x80000000U | PPN(kbasepgdir);
	write_csr(satp, atp);
	set_csr(sstatus, SSTATUS_SUM);
	printk("pmap.c:\t risc-v vm init success, now in virtual address mode.\n");
}

/* Overview:
 *   Initialize page structure and memory free list. The 'pages' array has one 'struct Page' entry
 * per physical page. Pages are reference counted, and free pages are kept on a linked list.
 *
 * Hint: Use 'LIST_INSERT_HEAD' to insert free pages to 'page_free_list'.
 */
void page_init(void) {
	/* Step 1: Initialize page_free_list. */
	/* Hint: Use macro `LIST_INIT` defined in include/queue.h. */
	/* Exercise 2.3: Your code here. (1/4) */
	LIST_INIT(&page_free_list);

	/* Step 2: Align `freemem` up to multiple of BY2PG. */
	/* Exercise 2.3: Your code here. (2/4) */
	freemem = ROUND(freemem, BY2PG);

	/* Step 3: Mark all memory below `freemem` as used (set `pp_ref` to 1) */
	/* Exercise 2.3: Your code here. (3/4) */
	int n_used_pages = PPN(freemem) - PPNOFFSET;
	for (u_long i = 0; i < n_used_pages; i++) {
		pages[i].pp_ref = 1;
	}

	/* Step 4: Mark the other memory as free. */
	/* Exercise 2.3: Your code here. (4/4) */
	for (u_long i = n_used_pages; i < npage; i++) {
		//pages[i].pp_ref = 0;
		LIST_INSERT_HEAD(&page_free_list, &pages[i], pp_link);
	}
}

/* Overview:
 *   Allocate a physical page from free memory, and fill this page with zero.
 *
 * Post-Condition:
 *   If failed to allocate a new page (out of memory, there's no free page), return -E_NO_MEM.
 *   Otherwise, set the address of the allocated 'Page' to *pp, and return 0.
 *
 * Note:
 *   This does NOT increase the reference count 'pp_ref' of the page - the caller must do these if
 *   necessary (either explicitly or via page_insert).
 *
 * Hint: Use LIST_FIRST and LIST_REMOVE defined in include/queue.h.
 */
int page_alloc(struct Page **new) {
	/* Step 1: Get a page from free memory. If fails, return the error code.*/
	struct Page *pp;
	/* Exercise 2.4: Your code here. (1/2) */
	if (LIST_EMPTY(&page_free_list)) {
		return -E_NO_MEM;
	}
	pp = LIST_FIRST(&page_free_list);

	LIST_REMOVE(pp, pp_link);

	/* Step 2: Initialize this page with zero.
	 * Hint: use `memset`. */
	/* Exercise 2.4: Your code here. (2/2) */
	memset((void *)page2kva(pp), 0, BY2PG);

	*new = pp;
	return 0;
}

/* Overview:
 *   Release a page 'pp', mark it as free.
 *
 * Pre-Condition:
 *   'pp->pp_ref' is '0'.
 */
void page_free(struct Page *pp) {
	assert(pp->pp_ref == 0);
	/* Just insert it into 'page_free_list'. */
	/* Exercise 2.5: Your code here. */
	LIST_INSERT_HEAD(&page_free_list, pp, pp_link);

}

/* Overview:
 *   Given 'pgdir', a pointer to a page directory, 'pgdir_walk' returns a pointer to the page table
 *   entry (with permission PTE_V) for virtual address 'va'.
 *
 * Pre-Condition:
 *   'pgdir' is a two-level page table structure.
 *
 * Post-Condition:
 *   If we're out of memory, return -E_NO_MEM.
 *   Otherwise, we get the page table entry, store
 *   the value of page table entry to *ppte, and return 0, indicating success.
 *
 * Hint:
 *   We use a two-level pointer to store page table entry and return a state code to indicate
 *   whether this function succeeds or not.
 */
int pgdir_walk(Pde *pgdir, u_long va, int create, Pte **ppte) {
	Pde *pgdir_entryp;
	struct Page *pp;

	/* Step 1: Get the corresponding page directory entry. */
	/* Exercise 2.6: Your code here. (1/3) */
	pgdir_entryp = pgdir + PDX(va);

	/* Step 2: If the corresponding page table is not existent (valid) and parameter `create`
	 * is set, create one. Set the permission bits 'PTE_V' for this new page in the
	 * page directory.
	 * If failed to allocate a new page (out of memory), return the error. */
	/* Exercise 2.6: Your code here. (2/3) */
	if (!(*pgdir_entryp & PTE_V)) {
		if (create) {
			try(page_alloc(&pp));
			pp->pp_ref = 1;
			*pgdir_entryp = PADDR2PTE(page2pa(pp)) | PTE_V;
		}
		else {
			*ppte = NULL;
			return 0;
		}
	}

	/* Step 3: Assign the kernel virtual address of the page table entry to '*ppte'. */
	/* Exercise 2.6: Your code here. (3/3) */
	*ppte = (Pte *)PTE2PADDR(*pgdir_entryp) + PTX(va);

	return 0;
}

/* Overview:
 *   Map the physical page 'pp' at virtual address 'va'. The permission (the low 12 bits) of the
 *   page table entry should be set to 'perm|PTE_V'.
 *
 * Post-Condition:
 *   Return 0 on success
 *   Return -E_NO_MEM, if page table couldn't be allocated
 *
 * Hint:
 *   If there is already a page mapped at `va`, call page_remove() to release this mapping.
 *   The `pp_ref` should be incremented if the insertion succeeds.
 */
int page_insert(Pde *pgdir, u_int asid, struct Page *pp, u_long va, u_int perm) {
	Pte *pte;

	if(va >= ULIM) panic("trying to map the kernel space, asid = %u\n", asid);

	/* Step 1: Get corresponding page table entry. */
	pgdir_walk(pgdir, va, 0, &pte);

	if (pte && (*pte & PTE_V)) {
		if (pa2page(PTE2PADDR(*pte)) != pp) {
			page_remove(pgdir, asid, va);
		} else {
			*pte = ((*pte) & ~0x3FF) | perm | PTE_A | PTE_D | PTE_R | PTE_V;
			tlb_invalidate(asid, va);
			return 0;
		}
	}

	/* Step 3: Re-get or create the page table entry. */
	/* If failed to create, return the error. */
	/* Exercise 2.7: Your code here. (2/3) */
	try(pgdir_walk(pgdir, va, 1, &pte));

	/* Step 4: Insert the page to the page table entry with 'perm | PTE_V' and increase its
	 * 'pp_ref'. */
	/* Exercise 2.7: Your code here. (3/3) */
	++pp->pp_ref;
	*pte = PADDR2PTE(page2pa(pp)) | perm | PTE_A | PTE_D | PTE_R | PTE_V;
	tlb_invalidate(asid, va);

	return 0;
}

/*Overview:
    Look up the Page that virtual address `va` map to.
  Post-Condition:
    Return a pointer to corresponding Page, and store it's page table entry to *ppte.
    If `va` doesn't mapped to any Page, return NULL.*/
struct Page *page_lookup(Pde *pgdir, u_long va, Pte **ppte) {
	struct Page *pp;
	Pte *pte;

	/* Step 1: Get the page table entry. */
	pgdir_walk(pgdir, va, 0, &pte);

	/* Hint: Check if the page table entry doesn't exist or is not valid. */
	if (pte == NULL || (*pte & PTE_V) == 0) {
		return NULL;
	}

	/* Step 2: Get the corresponding Page struct. */
	/* Hint: Use function `pa2page`, defined in include/pmap.h . */
	pp = pa2page(PTE2PADDR(*pte));
	if (ppte) {
		*ppte = pte;
	}

	return pp;
}

/* Overview:
 *   Decrease the 'pp_ref' value of Page 'pp'.
 *   When there's no references (mapped virtual address) to this page, release it.
 */
void page_decref(struct Page *pp) {
	assert(pp->pp_ref > 0);

	/* If 'pp_ref' reaches to 0, free this page. */
	if (--pp->pp_ref == 0) {
		page_free(pp);
	}
}

// Overview:
//   Unmap the physical page at virtual address 'va'.
void page_remove(Pde *pgdir, u_int asid, u_long va) {
	Pte *pte;

	/* Step 1: Get the page table entry, and check if the page table entry is valid. */
	struct Page *pp = page_lookup(pgdir, va, &pte);
	if (pp == NULL) {
		return;
	}

	/* Step 2: Decrease reference count on 'pp'. */
	page_decref(pp);

	/* Step 3: Flush TLB. */
	*pte = 0;
	tlb_invalidate(asid, va);
	return;
}

static void passive_alloc(u_int va, Pde *pgdir, u_int asid) {
	struct Page *p = NULL;

	if (va < UTEMP) {
		panic("address too low");
	}

	if (va >= USTACKTOP && va < USTACKTOP + BY2PG) {
		panic("invalid memory");
	}

	if (va >= UENVS && va < UPAGES) {
		panic("envs zone");
	}

	if (va >= UPAGES && va < UVPT) {
		panic("pages zone");
	}

	if (va >= ULIM) {
		panic("kernel address");
	}

	panic_on(page_alloc(&p));
	panic_on(page_insert(pgdir, asid, p, va, PTE_W | PTE_U));
}

void _do_tlb_refill(u_long va, u_int asid) {
	Pte *pte;
	struct Page *pp = page_lookup(cur_pgdir, va, &pte);
	if (pp == NULL) {
		passive_alloc(va, cur_pgdir, asid);
		// page_lookup(cur_pgdir, va, &pte);
	}
}