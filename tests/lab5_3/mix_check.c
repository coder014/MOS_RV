#include "../../fs/serv.h"
#include <lib.h>

void free_block(u_int);
u_int diskaddr(u_int);
void unmap_block(u_int);

int strecmp(char *a, char *b) {
	while (*b) {
		if (*a++ != *b++) {
			return 1;
		}
	}

	return 0;
}

static char *msg = "This is the NEW message of the day!\n";

void test_ide() {
	int data[4096], read[4096];
	int i;
	for (i = 0; i < 4096; i++) {
		data[i] = i;
	}
	ide_write(0, 4096, data, 32);
	ide_read(0, 4096, read, 32);
	for (i = 0; i < 4096; i++) {
		if (data[i] != read[i]) {
			user_panic("ide read/write is wrong");
		}
	}
	debugf("TEST IDE read/write test passed!\n");
}

void test_fs() {
	fs_init();
	int i, r, alloced_block[512];
	void *blk;
	u_int *bits;
	struct File *f;
	// back up bitmap
	if ((r = syscall_mem_alloc(0, (void *)UTEMP, PTE_W)) < 0) {
		user_panic("syscall_mem_alloc: %e", r);
	}
	bits = (u_int *)UTEMP;
	memcpy(bits, bitmap, BY2PG);
	// alloc_block
	for (i = 0; i < 512; i++) {
		if ((r = alloc_block()) < 0) {
			user_panic("alloc_block return: %d", r);
		}
		alloced_block[i] = r;
	}
	// block used to be free but not free any more
	for (i = 0; i < 512; i++) {
		r = alloced_block[i];
		u_int va = 0x10000000 + (r << 12);
		if (!((vpd[PDX(va)] & (PTE_V)) && (vpt[VPN(va)] & (PTE_V)))) {
			user_panic("block map is wrong");
		}
		user_assert(bits[r / 32] & (1 << (r % 32)));
		user_assert(!(bitmap[r / 32] & (1 << (r % 32))));
	}
	debugf("alloc_block is good!\n");
	// unmap block
	for (i = 0; i < 256; i++) {
		r = alloced_block[i];
		unmap_block(r);
		user_assert(!(bitmap[r / 32] & (1 << (r % 32))));
	}
	debugf("unmap_block is good!\n");
	// diskaddr
	for (i = 0; i < 512; i++) {
		if (diskaddr(i) != 0x10000000 + (i << 12)) {
			user_panic("diskaddr is incorrect");
		}
	}
	debugf("diskaddr is good!\n");
	// free block
	for (i = 0; i < 512; i++) {
		r = alloced_block[i];
		free_block(r);
		user_assert(bits[r / 32] & (1 << (r % 32)));
	}
	debugf("free_block is good!\n");
	// dir look up
	if ((r = file_open("/aaa", &f)) < 0 && r != -E_NOT_FOUND) {
		user_panic("file_open /aaa return wrong value: %d", r);
	} else if (r == 0) {
		user_panic("file_open non-existent file /aaa");
	}
	if ((r = file_open("/newmotd", &f)) < 0) {
		user_panic("file_open /newmotd: %d", r);
	}
	debugf("file_open is good!\n");
	// check file data
	if ((r = file_get_block(f, 0, &blk)) < 0) {
		user_panic("file_get_block: %d", r);
	}
	if (strecmp(blk, msg) != 0) {
		user_panic("file_get_block returned wrong data");
	}
	debugf("TEST fs test passed!\n");
}

int main() {
	ide_init();
	test_ide();
	test_fs();
	return 0;
}
