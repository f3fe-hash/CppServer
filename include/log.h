#ifndef __LOG_H__
#define __LOG_H__

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>

#include <string.h>

#include <unistd.h>

#define LOG_DIR "log/"

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

void server_log(Log_t* log, const char* msg, ...);
void open_log(Log_t** log, const char* name);

#ifdef __cplusplus
}
#endif

#endif