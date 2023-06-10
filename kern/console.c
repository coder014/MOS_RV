#include <sbi.h>

void printcharc(char ch) {
	sbi_call(SBI_CONSOLE_PUTCHAR, ch, 0, 0);
}

char scancharc(void) {
	return sbi_call(SBI_CONSOLE_GETCHAR, 0, 0, 0).value;
}

void halt(void) {
	sbi_call(SBI_SHUTDOWN, 0, 0, 0);
}
