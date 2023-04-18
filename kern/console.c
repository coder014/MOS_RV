#include <sbi.h>

void printcharc(char ch) {
	sbi_call(SBI_CONSOLE_PUTCHAR, ch, 0, 0);
}

char scancharc(void) {
	// not implemented yet
	return '\0';
}

void halt(void) {
	sbi_call(SBI_SHUTDOWN, 0, 0, 0);
}
