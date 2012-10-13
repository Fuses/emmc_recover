#ifndef DEVICE_H_
#define DEVICE_H_

int reset_device_pbl(void);
int qdload_device_connected(void);
int wait_device(const char* device);
int wait_device_gone(const char* device);
int check_file(const char* file);

#endif
