#ifndef EMMC_RECOVER_H_
#define EMMC_RECOVER_H_

#define VERSION "0.2"

int backup_partition(const char* partition, const char* filename);
int wait_device(const char* device);

#endif /* EMMC_RECOVER_H_ */
