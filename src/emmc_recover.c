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
	printf("\t-r | --reboot_pbl\n");

}

int main(int argc, const char **argv, char **env) {

	const char* device;
	void* options = NULL;

	int chunk_size = 0;

	if (argc <= 1) {
		usage();
		return EXIT_SUCCESS;
	}

	options = gopt_sort( & argc, argv, gopt_start(
		  gopt_option( 'h', 0, gopt_shorts( 'h' )			, gopt_longs( "help")),						// Help
		  gopt_option( 'b', GOPT_ARG,  gopt_shorts( 'b' )	, gopt_longs( "backup")),					// Just backups partition
		  gopt_option( 'f', GOPT_ARG,  gopt_shorts( 'f' )	, gopt_longs( "flash")),					// Flash image to --device
		  gopt_option( 'd', GOPT_ARG,  gopt_shorts( 'd' )	, gopt_longs( "device")),					// Device to use
		  gopt_option( 'c', GOPT_ARG,  gopt_shorts( 'c' )	, gopt_longs( "chunksize")),				// Chunksize
		  gopt_option( 'r', 0,  	   gopt_shorts( 'r' )	, gopt_longs( "reboot_pbl"))				// Reboot
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

	if (gopt(options, 'r')) {
		if (!module_loaded("qcserial") && !module_loaded("usbserial")) {
			load_usbserial();
			sleep(1);
		}
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

	gopt_arg(options, 'd', &device);

	printf("Messing up with device %s, ARE YOU SURE?\nCTRL+C if not, ENTER to continue\n", device);
	getc(stdin);

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
		flash_part_dd(device, imagefile);
		gopt_free(options);
		return EXIT_SUCCESS;

	}

	if (gopt(options, 'f') && gopt(options, 'c')) {
		const char* imagefile;
<<<<<<< HEAD
		long int written = 0;

		if (!qdload_device_connected()) {
			printf("Please connected bricked device\n");
			gopt_free(options);
			return EXIT_FAILURE;
		}

/*

		No need to blacklist qcserial. it's easier to check for the ttyUSB as qcseial
                becasue this way usb modems will not show up.


		if (module_loaded("qcserial")) {
			printf("Please blacklist qcserial module\n");
			gopt_free(options);
			return EXIT_FAILURE;
		}


		if (module_loaded("usbserial")) {
			remove_usbserial();
			sleep(1);
			load_usbserial();
			sleep(1);
		}
*/

		if (!qdload_device_connected() ){
			printf("Cannot find bricked device node\n");
			gopt_free(options);
			return EXIT_FAILURE;
		}

=======
>>>>>>> dcdf4a33787d7cdc72a4fa74e43d6887a0411f1f
		gopt_arg(options, 'f', &imagefile);
		if (!check_file(imagefile)) {
			printf("Cannot read image file %s\n", imagefile);
			gopt_free(options);
			return EXIT_FAILURE;
		}

		flash_part_chunk(device, imagefile, chunk_size);

<<<<<<< HEAD
		printf("Ok, ready to flash\n");
		printf("Filesize: \t%lu\n", filesize);
		printf("Chunksize is:\t%d\n", chunk_size);
		printf("Cycle count is: %f\n", c);

		printf("Press ENTER if everything is correct, CTRL+C if not\n");
		getc(stdin);

		FILE *image = fopen(imagefile, "rb");
		if (image == NULL) {
			printf("Cannot open file %s\n", imagefile);
			return EXIT_SUCCESS;
		}

		int readed;
		while( (readed = fread(chunk, 1, chunk_size, image)) != 0) {
			if (ferror(image)) {
				printf("Error while reading %s file\n", imagefile);
				fclose(image);
				return EXIT_FAILURE;
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
					if (qdload_device_connected()) break;
				}

				if (wi >= WAIT_TIMEOUT) {
					printf("Failed to detect device in dload-mode\n");
					return EXIT_FAILURE;
				}
				printf("Detected mode-switch\n");
				sleep(2);
			}

		}


		fclose(image);
		reset_device_pbl();
=======
		gopt_free(options);
		return EXIT_SUCCESS;
>>>>>>> dcdf4a33787d7cdc72a4fa74e43d6887a0411f1f

	}

	return EXIT_SUCCESS;
}

