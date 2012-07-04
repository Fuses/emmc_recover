#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdint.h>
#include <string.h>

#include "device.h"

int reset_device_pbl(void) {
						// Poweroff
	uint8_t msg[5] = {0x7E, 0x0E, 0x06, 0x19, 0x7E};

	int fd;
	int wrote;

	fd = open("/dev/ttyUSB0", O_RDWR | O_SYNC, 0600);
	if (fd == -1) {
		printf("Cannot reset device\n");
		return -1;
	}
	wrote = write(fd, msg, 5);
	close(fd);
	printf("Wrote %d bytes\n", wrote);
	return 0;
}

#define OP_LEN 1024
// Easier to pipe output from lsusb :)
int qdload_device_connected(void) {
	FILE *fp;
	char output[OP_LEN];
	memset(output, 0x00, OP_LEN);

	fp = popen("lsusb", "r");
	if (fp == NULL) {
		printf("FATAL ERROR: Cannot read usb devices\n");
		return 0;
	}

	while (fgets(output, OP_LEN, fp) != NULL ) {
		if (strstr(output, "05c6:9008") != NULL) {
			pclose(fp);
			return 1;
		}
	}

	pclose(fp);
	return 0;

}
