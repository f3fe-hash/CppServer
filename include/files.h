#ifndef __FILES_H__
#define __FILES_H__

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <sys/types.h>

#ifdef __cplusplus
extern "C"
{
#endif

FILE* open_file(const char* filename, const char* mode) __THROW __nonnull((1, 2));

ssize_t read_file  (const char* filename, char* buff, ssize_t size) __THROW __nonnull((1, 2));
ssize_t write_file (const char* filename, char* buff, ssize_t size) __THROW __nonnull((1, 2));
ssize_t read_filef (FILE* file, char* buff, ssize_t size) __THROW __nonnull((1, 2));
ssize_t write_filef(FILE* file, char* buff, ssize_t size) __THROW __nonnull((1, 2));

char* resolve_filename(const char* filename) __THROW __nonnull((1));

#ifdef __cplusplus
}
#endif

#endif