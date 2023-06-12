/*
 * operations on IDE disk.
 * reference: https://github.com/mit-pdos/xv6-riscv/blob/riscv/kernel/virtio_disk.c
 * see also: https://github.com/qemu/qemu/blob/v7.2.0/hw/block/virtio-blk.c
 * see also: https://github.com/qemu/qemu/blob/v7.2.0/hw/virtio/virtio.c
 */

#include "serv.h"
#include <lib.h>
#include <mmu.h>
#include <virtio.h>

#define VA2PA(va) (PTE2PADDR(vpt[VPN(va)]) | ((u_long)(va) & 0xFFF))
#define MMIO_IN(__r) syscall_read_dev(VIRTIO0 | (__r))
#define MMIO_OUT(__r, value) syscall_write_dev(VIRTIO0 | (__r), (value))

static u_char buf[BY2PG] __attribute__((aligned(BY2PG)));
static u_char disk_buf[BY2PG] __attribute__((aligned(BY2PG)));

static struct _disk {
	// a set (not a ring) of DMA descriptors, with which the
	// driver tells the device where to read and write individual
	// disk operations. there are NUM descriptors.
	// most commands consist of a "chain" (a linked list) of a couple of
	// these descriptors.
	struct virtq_desc *desc;

	// a ring in which the driver writes descriptor numbers
	// that the driver would like the device to process.  it only
	// includes the head descriptor of each chain. the ring has
	// NUM elements.
	struct virtq_avail *avail;

	// a ring in which the device writes descriptor numbers that
	// the device has finished processing (just the head of each chain).
	// there are NUM used ring entries.
	volatile struct virtq_used *used;

	// track info about in-flight operations,
	// for use when completion interrupt arrives.
	// indexed by first descriptor index of chain.
	u_short used_idx;
	volatile u_char status;

	// disk command headers.
	// one-for-one with descriptors, for convenience.
	struct virtio_blk_req *op;

} disk;

void ide_init()
{
	u_int status = 0;

	// check device
	if(MMIO_IN(VIRTIO_MMIO_MAGIC_VALUE) != 0x74726976 ||
        MMIO_IN(VIRTIO_MMIO_VERSION) != 2 ||
        MMIO_IN(VIRTIO_MMIO_DEVICE_ID) != 2 ||
        MMIO_IN(VIRTIO_MMIO_VENDOR_ID) != 0x554d4551) {
    	user_halt("could not find virtio disk");
	}
	// reset device
	MMIO_OUT(VIRTIO_MMIO_STATUS, status);
	// set ACKNOWLEDGE status bit
	status |= VIRTIO_CONFIG_S_ACKNOWLEDGE;
	MMIO_OUT(VIRTIO_MMIO_STATUS, status);
	// set DRIVER status bit
	status |= VIRTIO_CONFIG_S_DRIVER;
	MMIO_OUT(VIRTIO_MMIO_STATUS, status);

	// negotiate features
	MMIO_OUT(VIRTIO_MMIO_DEVICE_FEATURES_SEL, 0);
	u_int features = MMIO_IN(VIRTIO_MMIO_DEVICE_FEATURES);
	features &= ~(1 << VIRTIO_BLK_F_RO);
	features &= ~(1 << VIRTIO_BLK_F_SCSI);
	features &= ~(1 << VIRTIO_BLK_F_CONFIG_WCE);
	features &= ~(1 << VIRTIO_BLK_F_MQ);
	features &= ~(1 << VIRTIO_F_ANY_LAYOUT);
	features &= ~(1 << VIRTIO_RING_F_EVENT_IDX);
	features &= ~(1 << VIRTIO_RING_F_INDIRECT_DESC);
	MMIO_OUT(VIRTIO_MMIO_DEVICE_FEATURES, features);

	// tell device that feature negotiation is complete.
	status |= VIRTIO_CONFIG_S_FEATURES_OK;
	MMIO_OUT(VIRTIO_MMIO_STATUS, status);
	// re-read status to ensure FEATURES_OK is set.
	status = MMIO_IN(VIRTIO_MMIO_STATUS);
	if(!(status & VIRTIO_CONFIG_S_FEATURES_OK))
		user_halt("virtio disk FEATURES_OK unset");
	// initialize queue 0.
	MMIO_OUT(VIRTIO_MMIO_QUEUE_SEL, 0);
	// ensure queue 0 is not in use.
	if(MMIO_IN(VIRTIO_MMIO_QUEUE_READY))
		user_halt("virtio disk queue should not be ready");
	u_int max = MMIO_IN(VIRTIO_MMIO_QUEUE_NUM_MAX);
	if(max < NUM)
		user_halt("virtio disk max queue too short");

	// allocate and zero queue memory.
	disk.desc = &buf[0];
	disk.avail = &buf[1024];
	disk.used = &buf[2048];
	disk.op = &buf[3072];
	memset(buf, 0, BY2PG);

	// inform the device not to use interrupt
	disk.avail->flags = 1;

	// set queue size.
	MMIO_OUT(VIRTIO_MMIO_QUEUE_NUM, NUM);
	// write physical addresses.
	MMIO_OUT(VIRTIO_MMIO_QUEUE_DESC_LOW, VA2PA(disk.desc));
	MMIO_OUT(VIRTIO_MMIO_QUEUE_DESC_HIGH, 0);
	MMIO_OUT(VIRTIO_MMIO_DRIVER_DESC_LOW, VA2PA(disk.avail));
	MMIO_OUT(VIRTIO_MMIO_DRIVER_DESC_HIGH, 0);
	MMIO_OUT(VIRTIO_MMIO_DEVICE_DESC_LOW, VA2PA(disk.used));
	MMIO_OUT(VIRTIO_MMIO_DEVICE_DESC_HIGH, 0);
	// queue is ready
	MMIO_OUT(VIRTIO_MMIO_QUEUE_READY, 1);

	// tell device we're completely ready.
  	status |= VIRTIO_CONFIG_S_DRIVER_OK;
	MMIO_OUT(VIRTIO_MMIO_STATUS, status);

	disk.op->reserved = 0;
	disk.desc[0].addr = VA2PA(disk.op);
	disk.desc[0].len = sizeof(struct virtio_blk_req);
	disk.desc[0].flags = VRING_DESC_F_NEXT;
	disk.desc[0].next = 1;

	disk.desc[1].addr = VA2PA(&disk_buf);
	disk.desc[1].len = BY2PG;
	disk.desc[1].next = 2;

	disk.desc[2].addr = VA2PA(&disk.status);
	disk.desc[2].len = 1;
	disk.desc[2].flags = VRING_DESC_F_WRITE;
	disk.desc[2].next = 0;
#ifdef MOS_DEBUG
	debugf("ide_init succeeded!\n");
#endif
}

static void virtio_disk_rw(u_int secno, int write)
{
	if(write) disk.op->type = VIRTIO_BLK_T_OUT;
	else disk.op->type = VIRTIO_BLK_T_IN;
	disk.op->sector = secno;

	if(write) disk.desc[1].flags = 0;
	else disk.desc[1].flags = VRING_DESC_F_WRITE;
	disk.desc[1].flags |= VRING_DESC_F_NEXT;

	disk.status = 0xff; // device writes 0 on success

	// tell the device the first index in our chain of descriptors.
	disk.avail->ring[disk.avail->idx % NUM] = 0;
	disk.avail->idx += 1;
	MMIO_OUT(VIRTIO_MMIO_QUEUE_NOTIFY, 0); // value is queue number

	do {
		// debugf("waiting for disk IO to finish.... %u\n", disk.used->idx);
	} while(disk.used_idx + 1 != disk.used->idx);
	user_assert(++disk.used_idx == disk.used->idx);
	user_assert(disk.used->ring[disk.used_idx % NUM].id == 0);
	user_assert(disk.status == VIRTIO_BLK_S_OK);

	// memset(disk.desc, 0, 3 * sizeof(struct virtq_desc));
}

void ide_read(u_int diskno, u_int secno, void *dst, u_int nsecs) {
	user_assert(nsecs % SECT2BLK == 0);
	for (u_int i = 0; i < nsecs; i+=SECT2BLK) {
		virtio_disk_rw(secno + i, 0);
		memcpy(dst + (i * BY2SECT), disk_buf, BY2PG);
	}
}

void ide_write(u_int diskno, u_int secno, void *src, u_int nsecs) {
	user_assert(nsecs % SECT2BLK == 0);
	for (u_int i = 0; i < nsecs; i+=SECT2BLK) {
		memcpy(disk_buf, src + (i * BY2SECT), BY2PG);
		virtio_disk_rw(secno + i, 1);
	}
}
