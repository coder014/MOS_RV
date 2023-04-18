#include <sbi.h>

void printcharc(char ch) {
	sbi_call(SBI_CONSOLE_PUTCHAR, ch, 0, 0);
}

char scancharc(void) {
	// return *((volatile char *)(KSEG1 + DEV_CONS_ADDRESS + DEV_CONS_PUTGETCHAR));
	return '\0';
}

void halt(void) {
	sbi_call(SBI_SHUTDOWN, 0, 0, 0);
}
