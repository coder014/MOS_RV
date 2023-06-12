/* int spawnl(char *prog, char *args, ...) {
	// Thanks to MIPS calling convention, the layout of arguments on the stack
	// are straightforward.
	return spawn(prog, &args);
}
*/
#include <asm/asm.h>
FEXPORT(spawnl)

END(spawnl)