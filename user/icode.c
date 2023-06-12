#include <lib.h>

int main() {
	int fd, n, r;
	char buf[512 + 1];
#ifdef MOS_DEBUG
	debugf("icode: open /motd\n");
#endif
	if ((fd = open("/motd", O_RDONLY)) < 0) {
		user_panic("icode: open /motd: %d", fd);
	}
#ifdef MOS_DEBUG
	debugf("icode: read /motd\n");
#endif
	while ((n = read(fd, buf, sizeof buf - 1)) > 0) {
		buf[n] = 0;
		debugf("%s\n", buf);
	}
#ifdef MOS_DEBUG
	debugf("icode: close /motd\n");
#endif
	close(fd);
#ifdef MOS_DEBUG
	debugf("icode: spawn /init\n");
#endif
	if ((r = spawnl("init.b", "init", "initarg1", "initarg2", NULL)) < 0) {
		user_panic("icode: spawn /init: %d", r);
	}
#ifdef MOS_DEBUG
	debugf("icode: exiting\n");
#endif
	return 0;
}
