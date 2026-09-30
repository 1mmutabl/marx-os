#pragma once

#include "types.h"

void ata_init(void);

int ata_read_sectors(uint32 lba, uint8 count, void *buffer);
int ata_write_sectors(uint32 lba, uint8 count, const void *buffer);
