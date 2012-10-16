#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdint.h>
#include <string.h>
#include <sys/stat.h>
#include <inttypes.h>

#include "qmmc_flash.h"
#include "modhandler.h"
#include "device.h"

#define WAIT_TIMEOUT 10

void write_chunk(const char* file, long int offset, uint8_t *data, int nof_bytes);
int copy_partition(const char* partition, const char* filename);

int backup_partition(const char *device, const char* backupfile) {

	if (wait_device(device)) {
		if (!backup_partition(device, backupfile)) {
			printf("Backup failed!!\n");
			return 0;
		}
	}

	return 1;

}

int flash_part_dd(const char *device, const char* imagefile) {

	if (!check_file(imagefile)) {
		printf("Cannot read image file %s\n", imagefile);
		return 0;
	}

	
	if (wait_device(device)) {
		char command[256];
		memset(command, 0x00, 256);
		sprintf(command, "dd if=%s of=%s bs=2048", imagefile, device);
		int ret = system(command);
		printf("Return code is %d\n", ret);

		if (ret == 0) printf("Okay\n");
		else printf("FAILED!!\n");
		return 1;
	}

	return 0;
}

int flash_part_chunk(const char *device, const char* imagefile, uint32_t chunk_size) {
	// Qualcomm High-Speed USB Download Mode is partially blocked, Device has read/write access only few milliseconds
	struct stat st;
	uint8_t chunk[chunk_size];
	unsigned long filesize;

	long int written = 0;

	if (!qdload_device_connected()) {
		printf("Please connect device\n");
		return 0;
	}

	if (module_loaded("qcserial")) {
		printf("Please blacklist qcserial module\n");
		return 0;
	}

	if (module_loaded("usbserial")) {
		remove_usbserial();
		sleep(1);
		load_usbserial();
		sleep(1);
	}

	if (!check_file("/dev/ttyUSB0")) {
		printf("Cannot find bricked device node\n");
		return 0;
	}

	stat(imagefile, &st);
	filesize = st.st_size;

	memset(chunk, 0x00, chunk_size);

	float c = ( (float) filesize / (float) chunk_size) + 0.5F;

	printf("Ok, ready to flash\n");
	printf("Filesize: \t%lu\n", filesize);
	printf("Chunksize is:\t%d\n", chunk_size);
	printf("Cycle count is: %f\n", c);

	
	FILE *image = fopen(imagefile, "rb");
	if (image == NULL) {
		printf("Cannot open file %s\n", imagefile);
		return 0;
	}

	int readed;
	while( (readed = fread(chunk, 1, chunk_size, image)) != 0) {
		if (ferror(image)) {
			printf("Error while reading %s file\n", imagefile);
			fclose(image);
			return 0;
		}

		printf("Ready to write %d bytes\n", readed);
		printf("Resetting device\n");
		fflush(stdout);
		reset_device_pbl();
		printf("Reset command sent\n");
		fflush(stdout);
		if (wait_device(device)) {
			int wi;
			write_chunk(device, written, chunk, readed);
			written += readed;
			fflush(stdout);

			printf("Waiting mode-switch\n");
			wait_device_gone(device);

			// Wait till ttyUSB appers (dload mode)
			for (wi=0;wi<WAIT_TIMEOUT;wi++) {
				sleep(1);
				if (check_file("/dev/ttyUSB0")) break;
			}

			if (wi >= WAIT_TIMEOUT) {
				printf("Failed to detect device in dload-mode\n");
				return 0;
			}
			printf("Detected mode-switch\n");
			sleep(2);
		}

	}


	fclose(image);
	reset_device_pbl();
	return 1;
}

void write_chunk(const char* file, long int offset, uint8_t *data, int nof_bytes) {
	FILE *part = fopen(file, "r+b");
	if ( part == NULL ) {
		printf("FATAL: Failed to open %s\n", file);
		exit(-1);
		return;
	}

	if ( fseek(part, offset, SEEK_SET ) == 0 ) {
		printf("Writing %d bytes to offset 0x%08lX\n", nof_bytes, offset);
		fwrite(data, 1, nof_bytes, part);
	}
	else {
		printf("FATAL: cannot fseek to offset\n");
		fclose(part);
		exit(-1);
	}

	fclose(part);
	// No point to detect failures, you don't get any if io block is applied!!
}

int copy_partition(const char* partition, const char* filename) {

	FILE *part;
	FILE *backup_file;
	char ch;

	printf("Backing up partition %s to file %s\n", partition, filename);

	part = fopen(partition, "rb");
	if (part == NULL) {
		fprintf(stderr, "Cannot open file %s\n",partition);
		return 0;
	}
	backup_file = fopen(filename, "wb");
	if (backup_file == NULL) {
		fprintf(stderr, "Cannot open file %s\n", filename);
		return 0;
	}

	while (!feof(part)) {
		ch = fgetc(part);
		if (ferror(part)) {
			fprintf(stderr, "Cannot read file %s\n", partition);
			return 0;
		}

		if (!feof(part)) {
			fputc(ch, backup_file);
			if (ferror(backup_file)) {
				fprintf(stderr, "Cannot write file %s\n", filename);
				return 0;
			}
		}
	}

	fclose(backup_file);
	fclose(part);

	printf("Backup ok\n");

	return 1;
}
