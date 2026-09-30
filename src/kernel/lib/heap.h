#pragma once

#include "types.h"

void heap_init(uint32 start, uint32 size);

void *kmalloc(size_t size);
void  kfree(void *ptr);
void *krealloc(void *ptr, size_t size);
void *kcalloc(size_t num, size_t size);

void heap_dump(void);
