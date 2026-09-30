#pragma once

#include "fs/fatfs/ff.h"
#include "types.h"

FRESULT fs_init(void);

FRESULT file_create(const char *Path);
uint8   file_exists(const char *Path);
FRESULT file_write(const char *Path, const void *Write, int Size);
FRESULT file_read(const char *Path, void *Read, int Size);
int     file_size(const char *Path);
FRESULT file_append(const char *Path, const void *Write, int Size);
FRESULT file_read_at(const char *Path, int Offset, void *Read, int Size);
FRESULT file_truncate(const char *Path, int NewSize);
FRESULT file_write_at(const char *Path, int Offset, const void *Write,
                      int Size);

uint8        dir_exists(const char *Path);
FRESULT      dir_create(const char *Path);
unsigned int dir_size(const char *Path);
FRESULT      dir_read(const char *Path, unsigned int DirSize,
                      char *Content[DirSize]);
