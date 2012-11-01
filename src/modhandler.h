#ifndef MODHANDLER_H_
#define MODHANDLER_H_

int module_loaded(char *module);
int load_usbserial();
int remove_usbserial();
void verify_module();

#endif
