#include <sbi.h>
#include <types.h>

struct sbiret sbi_call(u_int sbi_type, u_int arg0, u_int arg1, u_int arg2) {
	asm volatile (
		"mv x17, %[sbi_type]\n" // set a7 to EID
		"mv x10, %[arg0]\n" // set a0 to arg0
		"mv x11, %[arg1]\n"
		"mv x12, %[arg2]\n"
		"ecall"
		: "+r"(arg0), "+r"(arg1)
		: [sbi_type] "r"(sbi_type), [arg0] "r"(arg0), [arg1] "r"(arg1), [arg2] "r"(arg2)
		: "memory"
	);
}