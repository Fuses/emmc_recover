#ifndef EMMC_RECOVER_H_
#define EMMC_RECOVER_H_

#define VERSION "0.3 alpha 3"

int backup_partition(const char* partition, const char* filename);
int wait_device(const char* device);
void write_chunk(const char* file, long int offset, uint8_t *data, int nof_bytes);


#endif /* EMMC_RECOVER_H_ */
