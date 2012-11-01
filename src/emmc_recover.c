/*
    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/stat.h>
#include <stdint.h>
#include <math.h>

#include "emmc_recover.h"
#include "gopt.h"
#include "device.h"
#include "qmmc_flash.h"
#include "modhandler.h"

#define MAX_CHUNK 2097152

void usage() {
	printf("emmc_recover %s\n", VERSION);
	printf("usage: emmc_recover [OPTIONS]\n");
	printf("\t-h | --help: display this help\n");
	printf("\t-b | --backup\n");
	printf("\t-f | --flash\n");
	printf("\t-d | --device\n");
	printf("\t-c | --chunksize\n");
	printf("\t-q | --quiet\n");
	printf("\t-r | --reboot_pbl\n");
	printf("\t-H | --hexdump\n");

}

int main(int argc, const char **argv, char **env) {

	const char* device;
	void* options = NULL;

	int chunk_size = 0;
	int quiet_mode = 0;

	if (argc <= 1) {
		usage();
		return EXIT_SUCCESS;
	}

	options = gopt_sort( & argc, argv, gopt_start(
		  gopt_option( 'h', 0, gopt_shorts( 'h' )			, gopt_longs( "help")),						// Help
		  gopt_option( 'b', GOPT_ARG,  gopt_shorts( 'b' )	, gopt_longs( "backup")),					// Just backups partition
		  gopt_option( 'q', 0,		   gopt_shorts( 'q' )	, gopt_longs( "quiet")),					// Quiet mode (no user prompts)
		  gopt_option( 'f', GOPT_ARG,  gopt_shorts( 'f' )	, gopt_longs( "flash")),					// Flash image to --device
		  gopt_option( 'd', GOPT_ARG,  gopt_shorts( 'd' )	, gopt_longs( "device")),					// Device to use
		  gopt_option( 'c', GOPT_ARG,  gopt_shorts( 'c' )	, gopt_longs( "chunksize")),				// Chunksize
		  gopt_option( 'r', 0,  	   gopt_shorts( 'r' )	, gopt_longs( "reboot_pbl")),				// Reboot
		  gopt_option( 'H', GOPT_ARG,  gopt_shorts( 'H' )	, gopt_longs( "hexdump"))				// Chunksize
	));

	if (options == NULL) {
		printf("Cannot init options!!\n");
		return EXIT_FAILURE;
	}

	if (gopt(options, 'h')) {
		usage(options);
		gopt_free(options);
		return EXIT_SUCCESS;
	}

	printf("================= emmc_recover " VERSION " ==========================\n");
	printf("\n");

	if (getuid() != 0 && geteuid() != 0) {
		printf("Run as root\n");
		gopt_free(options);
		return EXIT_FAILURE;
	}

	if (gopt(options, 'r') && !gopt(options, 'H')) {
		verify_module();
		reset_device_pbl();
		gopt_free(options);
		return EXIT_SUCCESS;
	}

	if (!gopt(options, 'd')) {
		usage(options);
		gopt_free(options);
		return EXIT_FAILURE;
	}

	if (gopt(options, 'c')) {
		const char *c;
		gopt_arg(options, 'c', &c);
		chunk_size = atoi(c);
		if (chunk_size <= 0 || chunk_size > MAX_CHUNK) {
			printf("Not valid chunk_size\n");
			return EXIT_FAILURE;
		}

		printf("Using chunksize %d\n", chunk_size);
		fflush(stdout);

	}

	if (gopt(options, 'q')) {
		quiet_mode = 1;

	}

	gopt_arg(options, 'd', &device);

	if (quiet_mode == 0) {
		printf("Messing up with device %s, ARE YOU SURE?\nCTRL+C if not, ENTER to continue\n", device);
		getc(stdin);
	}

	if (gopt(options, 'H')) {
		const char* offsets;
		gopt_arg(options, 'H', &offsets);

		if (strlen(offsets) != 17 || strchr(offsets,':') == NULL) {
			printf("emmc_recover --hexdump 00000000:00000000\n");
			gopt_free(options);
			return EXIT_SUCCESS;

		}
		else {
			char offset1_c[9], offset2_c[9];
			long int offset1, offset2;
			long int byte_count;
			uint8_t *data;

			memset(offset1_c, 0x00, 9);
			memset(offset2_c, 0x00, 9);

			strncpy(offset1_c, offsets, 8);
			strncpy(offset2_c, offsets+9, 8);

			sscanf(offset1_c, "%08lX", &offset1);
			sscanf(offset2_c, "%08lX", &offset2);

			byte_count = offset2 - offset1 + 1;
			if (byte_count <= 0 || byte_count > 0x10000000) {
				printf("Failed\n");
				return EXIT_FAILURE;
			}

			data = malloc(byte_count);
			if (data == NULL) {
				printf("Out of mem\n");
				return EXIT_FAILURE;
			}

			if (gopt(options, 'r')) {
				printf("Resetting device\n");
				verify_module();
				if (!reset_device_pbl()) {
					gopt_free(options);
					return EXIT_FAILURE;
				}
				sleep(1);
			}

			if (wait_device(device)) {
				if (!read_bytes(device, data, offset1, byte_count)) {
					gopt_free(options);
					return EXIT_FAILURE;
				}
				print_hexdump(offset1, data, byte_count);
			}
			free(data);

		}

		gopt_free(options);
		return EXIT_SUCCESS;
	}

	if (gopt(options, 'b') && !gopt(options, 'c')) {
		const char* backupfile;
		gopt_arg(options, 'b', &backupfile);

		backup_partition(device, backupfile);
		gopt_free(options);
		return EXIT_SUCCESS;

	}

	if (gopt(options, 'b') && gopt(options, 'c')) {
		printf("Not yet\n");
		gopt_free(options);
		return EXIT_SUCCESS;
	}

	if (gopt(options, 'f') && !gopt(options, 'c')) {
		const char* imagefile;
		gopt_arg(options, 'f', &imagefile);
		flash_part_dd(device, imagefile, quiet_mode);
		gopt_free(options);
		return EXIT_SUCCESS;

	}

	if (gopt(options, 'f') && gopt(options, 'c')) {
		const char* imagefile;
		gopt_arg(options, 'f', &imagefile);
		if (!check_file(imagefile)) {
			printf("Cannot read image file %s\n", imagefile);
			gopt_free(options);
			return EXIT_FAILURE;
		}

		flash_part_chunk(device, imagefile, chunk_size, quiet_mode);

		gopt_free(options);
		return EXIT_SUCCESS;

	}

	gopt_free(options);

	return EXIT_SUCCESS;
}

void print_row(long int offset, uint8_t *row_bytes, int len) {
	int i;
	char c;

	printf("%08lX ", offset);

	for (i=0;i<len;i++) printf("%02X ", row_bytes[i]);
	if (len < 0xF) for (i=0;i<0xF-len+1;i++) printf("   ");

	printf("|");
	for (i=0;i<len;i++) {
		c = row_bytes[i];
		if (c >= 0x20 && c <= 0x7E) printf("%c", c);
		else printf(".");
	}
	printf("|\n");
	fflush(stdout);
}

void print_hexdump(long int offset, uint8_t *data, int len) {
	int i,j,rows,d;

	rows = (int) len/0x10;

	for (i=0;i<rows;i++){
		print_row(offset, data, 0x10);
		data += 0x10;
		offset += 0x10;
	}

	d = len - (rows * 0x10);
	if (d > 0) print_row(offset, data, d);

}

