#include <stackframe.h>

EXPORT(exc_gen_entry)
	SAVE_ALL
	mv a0, sp
.extern traps_dispatch
	jal traps_dispatch

.globl ret_from_exception;
ret_from_exception:
	RESTORE_ALL
	sret