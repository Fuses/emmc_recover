#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdint.h>
#include <string.h>
#include <sys/stat.h>

#include "device.h"

int reset_device_pbl(void) {

	int fd;
				// Poweroff
	uint8_t msg[] = {0x7E, 0x0E, 0x06, 0x19, 0x7E};

	fd = open("/dev/ttyUSB0", O_RDWR | O_SYNC, 0600);
	if (fd == -1) {
		printf("Cannot reset device\n");
		return 0;
	}
	write(fd, msg, sizeof(msg));
	close(fd);
	//printf("Wrote %d bytes\n", wrote);
	return 1;
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

int wait_device(const char* device) {

	if (check_file(device)) {
		printf("Device already exists!!\n");
		printf("Please disconnect device!!\n");
		return 0;
	}

	printf("Waiting device %s.......\n", device);

	while(1) {
		usleep(1);
		if (check_file(device)) {
			printf("Found device!\n");
			fflush(stdout);
			return 1;
		}
	}

}

int wait_device_gone(const char* device) {
	while (1) {
		if (!check_file(device)) {
			printf("Device changed mode\n");
			fflush(stdout);
			return 1;
		}
		sleep(1);
	}
}

int check_file(const char* file) {
	struct stat s;
	int ret = stat(file, &s);
	if (ret != 0) return 0;
	return 1;
}

