#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <termios.h>
#include <stdint.h>
#include <string.h>
#include <glob.h>
#include <string.h>


#include "device.h"


char* glob_pattern(char *dir, char *wildcard)
{
  char *gfilename;
  size_t cnt=0, length=0;
  glob_t glob_results;
  char **p;
  chdir(dir);

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
