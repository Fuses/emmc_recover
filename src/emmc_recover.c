/*
	Copyright (C) 2012 Fuses

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

#include "emmc_recover.h"
#include "gopt.h"

void usage() {
	printf("emmc_recovery %s usage:\n", VERSION);
	printf("emmc_recovery [OPTIONS]\n");
	printf("\t-h | --help: display this help\n");
	printf("\t-b | --backup\n");
	printf("\t-f | --flash\n");
	printf("\t-d | --device\n");
	printf("\t-a | --backupafter\n");
	printf("\nCopyright (C) 2012 Fuses, source released under GPL license\n");
}

int check_file(const char* file) {
	struct stat s;
	int ret = stat(file, &s);
	if (ret != 0) return 0;
	return 1;
}

int main(int argc, const char **argv, char **env) {

	const char* device;
	const char* backup_after_file;
	void* options = NULL;

	if (argc <= 1) {
		usage();
		return EXIT_SUCCESS;
	}

	options = gopt_sort( & argc, argv, gopt_start(
		  gopt_option( 'h', 0, gopt_shorts( 'h' )			, gopt_longs( "help")),			// Help
		  gopt_option( 'b', GOPT_ARG,  gopt_shorts( 'b' )	, gopt_longs( "backup")),		// Just backups partition
		  gopt_option( 'f', GOPT_ARG,  gopt_shorts( 'f' )	, gopt_longs( "flash")),		// Flash image to --device
		  gopt_option( 'a', GOPT_ARG,  gopt_shorts( 'a' )	, gopt_longs( "backupafter")),  // Backups same partition after flash
		  gopt_option( 'd', GOPT_ARG,  gopt_shorts( 'd' )	, gopt_longs( "device"))		// Device to use
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

	if (!gopt(options, 'd')) {
		usage(options);
		gopt_free(options);
		return EXIT_FAILURE;
	}

	printf("=== emmc_recover %s, written by Fuses ===== \n", VERSION);

	// If "Backup after flash flag" is set, check that file does not exists
	if (gopt(options, 'a')) {
		gopt_arg(options, 'a', &backup_after_file);
		if (check_file(backup_after_file)) {
			printf("File %s already exists!\n", backup_after_file);
			gopt_free(options);
			return EXIT_FAILURE;
		}
	}

	gopt_arg(options, 'd', &device);

	printf("Messing up with device %s, ARE YOU SURE?\nCTRL+C if not, ENTER to continue\n", device);
	getc(stdin);

	if (gopt(options, 'b')) {
		const char* backupfile;
		gopt_arg(options, 'b', &backupfile);

		if (wait_device(device)) {
			if (!backup_partition(device, backupfile)) {
				printf("Backup failed!!\n");
				gopt_free(options);
				return EXIT_FAILURE;
			}
		}
		gopt_free(options);
		return EXIT_SUCCESS;

	}

	if (gopt(options, 'f')) {
		const char* imagefile;
		gopt_arg(options, 'f', &imagefile);
		if (!check_file(imagefile)) {
			printf("Cannot read image file %s\n", imagefile);
			gopt_free(options);
			return EXIT_FAILURE;
		}

		printf("Flash image file is %s\nDevice is %s\n\nPress ENTER if everything is correct, CTRL+C if not\n", imagefile, device);
		getc(stdin);
		if (wait_device(device)) {
			// You can also do verifications at this point, but there are only 5 seconds time.......
			char command[256];
			memset(command, 0x00, 256);
			sprintf(command, "dd if=%s of=%s bs=2048", imagefile, device);
			int ret = system(command); // Can also use backup function to do this, but I wan't to see dd output in console :)
			printf("Return code is %d\n", ret);
			if (ret == 0 && gopt(options, 'a')) {
				gopt_arg(options, 'a', &backup_after_file);
				if (!backup_partition(device, backup_after_file)) {
					printf("Backup failed!!\n");
					gopt_free(options);
					return EXIT_FAILURE;
				}

			}

			gopt_free(options);
			if (ret == 0) printf("Okay\n");
			else printf("FAILED!!\n");
			return EXIT_SUCCESS;
		}
	}

	return EXIT_SUCCESS;
}

int wait_device(const char* device) {

	if (check_file(device)) {
		printf("Device already exists!!\n");
		printf("Please disconnect device!!\n");
		return 0;
	}

	printf("Waiting device %s.......\n", device);

	while(1) {
		usleep(1000);
		if (check_file(device)) {
			printf("Foundit!\n");
			fflush(stdout);
			return 1;
		}
	}

}

int backup_partition(const char* partition, const char* filename) {

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
