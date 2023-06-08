#include <asm/asm.h>
FEXPORT(env_pop_tf)
	li       t0, 0x80000000
	slli     a1, a1, 22
	or       t0, t0, a1
	or       t0, t0, a2
	csrw     satp, t0
	mv       sp, a0
.extern ret_from_exception
	j        ret_from_exception
END(env_pop_tf)