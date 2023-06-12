#include <sbi.h>

void printcharc(char ch) {
	sbi_call(SBI_CONSOLE_PUTCHAR, ch, 0, 0);
}

long scancharc(void) {
	/*
	 * According to SBI specification:
	 *  --->  long sbi_console_getchar(void)
	 * we should take the value from sbiret.error,
	 *   which is equivalent to use the value of a0.
	 */
	return sbi_call(SBI_CONSOLE_GETCHAR, 0, 0, 0).error;
}

void halt(void) {
	sbi_call(SBI_SHUTDOWN, 0, 0, 0);
}
