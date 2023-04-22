#ifndef _PMAP_H_
#define _PMAP_H_

#include <mmu.h>
#include <printk.h>
#include <queue.h>
#include <types.h>

extern Pde *cur_pgdir;

LIST_HEAD(Page_list, Page);
typedef LIST_ENTRY(Page) Page_LIST_entry_t;

struct Page {
	Page_LIST_entry_t pp_link; /* free list link */

	// Ref is the count of pointers (usually in page table entries)
	// to this page.  This only holds for pages allocated using
	// page_alloc.  Pages allocated at boot time using pmap.c's "alloc"
	// do not have valid reference count fields.

	u_short pp_ref;
};

extern struct Page *pages;
extern struct Page_list page_free_list;

#define PPNOFFSET 0x80000

static inline u_long page2ppn(struct Page *pp) {
	return (pp - pages) + PPNOFFSET;
}

static inline u_long page2pa(struct Page *pp) {
	return page2ppn(pp) << PGSHIFT;
}

static inline struct Page *pa2page(u_long pa) {
	if ((PPN(pa)-PPNOFFSET) >= npage) {
		panic("pa2page called with invalid pa: %x", pa);
	}
	return &pages[PPN(pa) - PPNOFFSET];
}

#define page2kva page2pa

static inline u_long va2pa(Pde *pgdir, u_long va) {
	Pte *p;

	pgdir = &pgdir[PDX(va)];
	if (!(*pgdir & PTE_V)) {
		return ~0;
	}
	p = (Pte *)PTE2PADDR(*pgdir);
	if (!(p[PTX(va)] & PTE_V)) {
		return ~0;
	}
	return PTE2PADDR(p[PTX(va)]);
}

void riscv_detect_memory(u_int);
void riscv_vm_init(void);
void riscv_init(void);
void page_init(void);
void *alloc(u_int n, u_int align, int clear);

int page_alloc(struct Page **pp);
void page_free(struct Page *pp);
void page_decref(struct Page *pp);
int page_insert(Pde *pgdir, u_int asid, struct Page *pp, u_long va, u_int perm);
struct Page *page_lookup(Pde *pgdir, u_long va, Pte **ppte);
void page_remove(Pde *pgdir, u_int asid, u_long va);
/* Overview:
 *   Invalidate the TLB entry with specified 'asid' and virtual address 'va'.
 */
#define tlb_invalidate() do {                   \
	asm volatile("sfence.vma" ::: "memory");    \
} while (0)

#endif /* _PMAP_H_ */
