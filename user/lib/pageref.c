#include <lib.h>

int pageref(void *v) {
	u_int pte;

	if (!(vpd[PDX(v)] & PTE_V)) {
		return 0;
	}

	pte = vpt[VPN(v)];

	if (!(pte & PTE_V)) {
		return 0;
	}

	return pages[PPN(PTE2PADDR(pte))].pp_ref;
}
