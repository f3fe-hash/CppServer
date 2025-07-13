#include "log.h"

void server_log(Log_t* log, const char* msg, ...)
{
    int n = 0;
    size_t size = 0;
    char* p = NULL;
    va_list ap;

    va_start(ap, msg);
    n = vsnprintf(p, size, msg, ap);
    va_end(ap);

    if (n < 0)
        return;

    size = (size_t) n + 1;
    p = malloc(size);

    if (p == NULL)
        return;

    va_start(ap, msg);
    n = vsnprintf(p, size, msg, ap);
    va_end(ap);

    if (n < 0)
    {
        free(p);
        return;
    }

    chdir(LOG_DIR);
    log->_log = fopen(log->name, "a");
    if (log->_log != NULL)
    {
        fprintf(log->_log, "%s\n", p);
        fclose(log->_log);
    }
    chdir("..");
    
    free(p);
}

void open_log(Log_t** log, const char* name)
{
    // Go to the directory
    chdir(LOG_DIR);

    // Allocate the log, and set its name
    *log = malloc(sizeof(Log_t));
    if (*log == NULL) return;
    (*log)->name = malloc(strlen(name) + 1);
    if ((*log)->name == NULL)
    {
        free(*log);
        *log = NULL;
        return;
    }
    strcpy((*log)->name, name);

    // Delete any existing log file
    remove(name);

    // Create the log
    (*log)->_log = fopen(name, "a");
    fclose((*log)->_log);

    chdir("..");
}