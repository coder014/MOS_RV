#include <pmap.h>
#include <printk.h>
#include <fdt.h>

/* These variables are set by mips_detect_memory() */
static u_long memsize; /* Maximum physical address */
u_long npage;	       /* Amount of memory(in pages) */

Pde *cur_pgdir;

struct Page *pages;
static u_long freemem;

struct Page_list page_free_list; /* Free list of physical pages */

/* Overview:
 *   Read memory size from DEV_MP to initialize 'memsize' and calculate the corresponding 'npage'
 *   value.
 */
void mips_detect_memory(u_int dtb) {
	/* Step 1: Initialize memsize. */
	memsize = query_mem_size(dtb);

	/* Step 2: Calculate the corresponding 'npage' value. */
	/* Exercise 2.1: Your code here. */
	npage = memsize / BY2PG;

	printk("Memory size: %lu KiB, number of pages: %lu\n", memsize / 1024, npage);
}
