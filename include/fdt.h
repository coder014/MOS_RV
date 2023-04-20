// LICENSE: https://github.com/riscv-software-src/riscv-pk/blob/master/machine/fdt.h

#ifndef __FDT_H_
#define __FDT_H_

#include <types.h>

#define FDT_MAGIC	0xd00dfeed
#define FDT_VERSION	17

struct fdt_header {
  u_int magic;
  u_int totalsize;
  u_int off_dt_struct;
  u_int off_dt_strings;
  u_int off_mem_rsvmap;
  u_int version;
  u_int last_comp_version; /* <= 17 */
  u_int boot_cpuid_phys;
  u_int size_dt_strings;
  u_int size_dt_struct;
};

#define FDT_BEGIN_NODE	1
#define FDT_END_NODE	2
#define FDT_PROP	3
#define FDT_NOP		4
#define FDT_END		9

struct fdt_scan_node {
  const struct fdt_scan_node *parent;
  const char *name;
  int address_cells;
  int size_cells;
};

struct fdt_scan_prop {
  const struct fdt_scan_node *node;
  const char *name;
  u_int *value;
  int len; // in bytes of value
};

struct fdt_cb {
  void (*open)(const struct fdt_scan_node *node, void *extra);
  void (*prop)(const struct fdt_scan_prop *prop, void *extra);
  void (*done)(const struct fdt_scan_node *node, void *extra); // last property was seen
  int  (*close)(const struct fdt_scan_node *node, void *extra); // -1 => delete the node + children
  void *extra;
};

void fdt_scan(u_int fdt, const struct fdt_cb *cb);

// Extract fields
const u_int *fdt_get_address(const struct fdt_scan_node *node, const u_int *base, u_int *value);
const u_int *fdt_get_size(const struct fdt_scan_node *node, const u_int *base, u_int *value);

u_int query_mem_size(u_int fdt);

#endif
