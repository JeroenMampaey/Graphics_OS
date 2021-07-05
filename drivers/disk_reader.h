#ifndef DISK_READER_H
#define DISK_READER_H

#include "../cpu/ports.h"

void read_disk(int LBA, unsigned char num_of_sectors, short* destination);

#endif