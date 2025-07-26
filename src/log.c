#include "log.h"

__THROW __nonnull((1)) int mkdir_p(const char* dir)
{
    char tmp[LOG_PATH_MAX_LENGTH];
    char* p = NULL;
    size_t len;

    len = strlen(dir);
    if (len >= sizeof(tmp))
        return -1;

    strcpy(tmp, dir);

    // Remove trailing slash if any
    if (tmp[len - 1] == '/')
        tmp[len - 1] = 0;

    for (p = tmp + 1; *p; p++)
    {
        if (*p == '/')
        {
            *p = 0;
            if (mkdir(tmp, 0755) != 0)
                if (errno != EEXIST) return -1;
            *p = '/';
        }
    }
    // Create final directory
    if (mkdir(tmp, 0755) != 0)
        if (errno != EEXIST)
            return -1;

    return 0;
}

__THROW __nonnull((1, 2)) void server_log(Log_t* log, const char* msg, ...)
{
#ifdef DEBUG
    printf("In server_log\n");
#endif
    char stack_buf[MAX_LOG_BUFFER];
    char* buf = stack_buf;
    int n;
    va_list ap;

    // Try writing to stack buffer
    va_start(ap, msg);
    n = vsnprintf(buf, MAX_LOG_BUFFER, msg, ap);
    va_end(ap);

    if (n < 0)
        return;

    // Message was too large for stack buffer, allocate heap buffer instead
    if ((size_t)n >= MAX_LOG_BUFFER)
    {
        printf("Doing heap allocation\n");

        buf = malloc(n + 1);
        if (!buf)
            return;

        va_start(ap, msg);
        vsnprintf(buf, n + 1, msg, ap);
        va_end(ap);
    }

    // Construct full path to log file
    char full_path[LOG_PATH_MAX_LENGTH];
    int len = snprintf(full_path, sizeof(full_path), "%s/%s", LOG_PATH, log->name);

    // Ensure that the path is correct
    if ((size_t)len >= LOG_PATH_MAX_LENGTH)
    {
        printf("Path was truncated\n");

        // Path was truncated - could log or silently fail
        if (buf != stack_buf) free(buf);
        return;
    }

    write_file(full_path, buf, strlen(buf));

    if (buf != stack_buf)
        free(buf);
}


__THROW __nonnull((1, 2)) void open_log(Log_t** log, const char* name)
{
#ifdef DEBUG
    printf("In open_log\n");
#endif

    // Allocate Log_t
    Log_t* new_log = malloc(sizeof(Log_t));
    if (!new_log) return;

    // Copy log name
    new_log->name = malloc(strlen(name) + 1);
    if (!new_log->name)
    {
#ifdef DEBUG
        printf("Name allocation failed\n");
#endif
        free(new_log);
        *log = NULL;
        return;
    }
    strcpy(new_log->name, name);

    // Handle path expansion for ~ in LOG_PATH
    char* expanded_log_path = (char*)resolve_filename(LOG_PATH);
    if (!expanded_log_path)
    {
#ifdef DEBUG
        printf("Failed to resolve log path\n");
#endif
        free(new_log->name);
        free(new_log);
        *log = NULL;
        return;
    }

    // Construct full file path (directory + filename)
    char full_path[LOG_PATH_MAX_LENGTH];
    int len = snprintf(full_path, sizeof(full_path), "%s/%s", expanded_log_path, name);
    if (len < 0 || (size_t)len >= sizeof(full_path))
    {
#ifdef DEBUG
        printf("Full path too long or snprintf error\n");
#endif
        free(new_log->name);
        free(new_log);
        *log = NULL;
        return;
    }

    // Create the directory if it doesn't exist
    if (mkdir_p(expanded_log_path) != 0)
    {
#ifdef DEBUG
        printf("Failed to create directory: %s\n", expanded_log_path);
#endif
        free(new_log->name);
        free(new_log);
        *log = NULL;
        return;
    }

    // Now open the file for append (create if doesn't exist)
    FILE* fp = fopen(full_path, "a");
    if (!fp)
    {
#ifdef DEBUG
        printf("Failed to open file '%s' for append: %s\n", full_path, strerror(errno));
#endif
        free(new_log->name);
        free(new_log);
        *log = NULL;
        return;
    }
    fclose(fp);

    new_log->_log = NULL;  // Not keeping file open here
    *log = new_log;
}