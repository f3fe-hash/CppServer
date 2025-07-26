#ifndef __LOG_H__
#define __LOG_H__

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>

#include <errno.h>
#include <unistd.h>

#include <sys/stat.h>
#include <sys/types.h>

#include "files.h"

//#define DEBUG

#define LOG_PATH            "~/.cppserver/CppServerFiles/log"
#define LOG_PATH_MAX_LENGTH 51 // 21 for LOG_PATH, 15 for home expansi0on, and 15 for filename
#define MAX_LOG_BUFFER      1024

#define COLOR_RED    "\033[31m"
#define COLOR_YELLOW "\033[33m"
#define COLOR_GREEN  "\033[32m"
#define COLOR_BLUE   "\033[34m"

#define COLOR_RESET "\033[0m"
#define COLOR_BOLD  "\033[1m"

#define OK(fmt, ...)   printf("%s[  OK  ] ", COLOR_GREEN); printf(fmt, ##__VA_ARGS__); printf("%s", COLOR_RESET)
#define INFO(fmt, ...) printf("%s[ INFO ] ", COLOR_YELLOW); printf(fmt, ##__VA_ARGS__); printf("%s", COLOR_RESET)
#define FATAL(fmt, ...) printf("%s%s[ FATL ] ", COLOR_RED, COLOR_BOLD); printf(fmt, ##__VA_ARGS__); printf("%s", COLOR_RESET)

#ifdef __cplusplus
extern "C"
{
#endif

typedef struct
{
    char* name;
    FILE* _log;
} Log_t;

int mkdir_p(const char* dir) __THROW __nonnull((1));

void server_log(Log_t* log, const char* msg, ...) __THROW __nonnull((1, 2));
void open_log(Log_t** log, const char* name) __THROW __nonnull((1, 2));

#ifdef __cplusplus
}
#endif

#endif