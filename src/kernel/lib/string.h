#pragma once

#include "types.h"

void *memset(void *dest, int value, uint32 count);
void *memcpy(void *dest, const void *src, uint32 count);
void *memmove(void *dest, const void *src, uint32 count);
int   memcmp(const void *a, const void *b, uint32 count);
char *strchr(const char *str, int character);
char *strcpy(char *dest, const char *src);
int   strlen(const char *str);
int   strcmp(const char *String, const char *Other);
char *strrchr(const char *str, int character);
int   snprintf(char *buffer, uint32 size, const char *format, ...);
char  parse_escape(char Character, char Next);
