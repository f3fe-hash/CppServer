#include "files.h"

__THROW __nonnull((1)) FILE* open_file(const char* filename, const char* mode)
{
    char* name = resolve_filename(filename);  // use char* (not const char*)
    if (!name)
        return NULL;

    FILE* fp = fopen(name, mode);

    if (!fp)
        printf("Error opening file %s\n", name);

    free(name);  // Free the allocated string

    return fp;
}

__THROW __nonnull((1, 2)) ssize_t read_file(const char* filename, char* buff, ssize_t size)
{
    FILE* fp = open_file(filename, "rb");
    if (!fp)
        return -1;

    ssize_t n = fread(buff, 1, size, fp);
    fclose(fp);  // Don't leak file handles
    return n;
}

__THROW __nonnull((1, 2)) ssize_t write_file(const char* filename, char* buff, ssize_t size)
{
    FILE* fp = open_file(filename, "wb");
    if (!fp)
        return -1;

    ssize_t n = fwrite(buff, 1, size, fp);
    fclose(fp);
    return n;
}

__THROW __nonnull((1, 2)) ssize_t read_filef(FILE* file, char* buff, ssize_t size)
{
    return fread(buff, 1, size, file);
}

__THROW __nonnull((1, 2)) ssize_t write_filef(FILE* file, char* buff, ssize_t size)
{
    return fwrite(buff, 1, size, file);
}


__THROW __nonnull((1)) char* resolve_filename(const char* filename)
{
    if (filename[0] == '~' && filename[1] == '/')
    {
        char* home = getenv("HOME");
        if (!home)
        {
#ifdef DEBUG
            printf("Environment variable HOME not set, cannot expand ~\n");
#endif
            return NULL;
        }

        size_t home_len = strlen(home);
        size_t rest_len = strlen(filename + 1); // skip '~'
        size_t total_len = home_len + rest_len + 1; // +1 for '\0'

        char* expanded = malloc(total_len);
        if (!expanded)
            return NULL;

        strcpy(expanded, home);
        strcat(expanded, filename + 1); // append rest of path after '~'

        return expanded;
    }
    else
    {
        // Manually copy string without strdup
        size_t len = strlen(filename);
        char* copy = malloc(len + 1);
        if (!copy)
            return NULL;

        strcpy(copy, filename);
        return copy;
    }
}