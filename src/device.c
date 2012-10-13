#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <termios.h>
#include <stdint.h>
#include <string.h>
<<<<<<< HEAD
#include <glob.h>
#include <string.h>

=======
#include <sys/stat.h>
>>>>>>> dcdf4a33787d7cdc72a4fa74e43d6887a0411f1f

#include "device.h"


char* glob_pattern(char *dir, char *wildcard)
{
  char *gfilename;
  size_t cnt=0, length=0;
  glob_t glob_results;
  char **p;
  chdir(dir);

<<<<<<< HEAD
  glob(wildcard, GLOB_NOCHECK, 0, &glob_results);

  /* How much space do we need?  */
  for (p = glob_results.gl_pathv, cnt = glob_results.gl_pathc;
       cnt; p++, cnt--)
    length += strlen(*p) + 1;

  /* Allocate the space and generate the list.  */
  gfilename = (char *) calloc(length, sizeof(char));
  for (p = glob_results.gl_pathv, cnt = glob_results.gl_pathc;
       cnt; p++, cnt--)
    {
      strcat(gfilename, *p);
      if (cnt > 1)
        strcat(gfilename, " ");
    }

  globfree(&glob_results);
  return gfilename;
}

int reset_device_pbl() {
    int serialfd;
    struct termios terminal_data;
    //char qdlresp[1024];
    char tty[32];

    
    printf("Waiting for device");
    while( (serialfd = open(tty, O_RDWR) ) == -1) {
	    printf(".");
	    fflush(stdout);
	    sleep(2);
	    sprintf(tty, "/dev/%s", glob_pattern("/sys/bus/usb-serial/drivers/qcserial", "ttyUSB*"));
    }


    printf(" - found %s - OK\n", tty);
    tcgetattr (serialfd, &terminal_data);
    cfmakeraw (&terminal_data);
    tcsetattr (serialfd, TCSANOW, &terminal_data);


    char seq[] = {0x7e, 0x0e, 0x06, 0x19, 0x7e}; // 0x0E
    write(serialfd, seq, 5);
    return 0;
=======
	fd = open("/dev/ttyUSB0", O_RDWR | O_SYNC, 0600);
	if (fd == -1) {
		printf("Cannot reset device\n");
		return 0;
	}
	write(fd, msg, sizeof(msg));
	close(fd);
	//printf("Wrote %d bytes\n", wrote);
	return 1;
>>>>>>> dcdf4a33787d7cdc72a4fa74e43d6887a0411f1f
}


#define OP_LEN 1024
// Easier to pipe output from lsusb :)
int qdload_device_connected(void) {

	if ( strlen(glob_pattern("/sys/bus/usb-serial/drivers/qcserial", "ttyUSB*")) > 7 ) {
		return 1;
	} else {
		return 0;
	}
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

