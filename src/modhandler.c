#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "modhandler.h"

#define LSZ 256

extern long delete_module(const char *, unsigned int);

int module_loaded(char *module) {
	FILE *file;
	char line[LSZ + 1];
	int module_loaded = 0;

	file = fopen("/proc/modules", "r");
	if (file == NULL) {
		printf("Cannot read modules\n");
		return -1;
	}

	memset(line, 0x00, LSZ+1);

	while (fgets (line, LSZ, file) != NULL) {
		if (strstr(line, module) != NULL) {
			module_loaded = 1;
			break;
		}
	}

	fclose(file);

	return module_loaded;
}

int load_usbserial() {
	return system("modprobe usbserial vendor=0x05c6 product=0x9008");
}

int remove_usbserial() {
	unsigned int flags = O_NONBLOCK|O_EXCL;
	return delete_module("usbserial", flags);
}
