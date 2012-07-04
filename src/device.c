#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdint.h>

#include "pbl_reset.h"

int reset_device_pbl() {
	uint8_t msg[4] = {0x7E, 0x0A, 0x00, 0x00}; // Need crc

	int fd;
	fd = open("/dev/ttyUSB0", O_RDWR | O_SYNC, 0600);
	if (fd == -1) {
			return -1;
	}
	write(fd, msg, 4);
	close(fd);
	sleep(2);
	return 0;
}
