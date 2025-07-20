#include "lexer.h"

const char** parse_str(const char* str, const char deliminator)
{
    // Count the number of deliminators
    int num_delim = 0;
    for (int i = 0; i < (int)strlen(str); i++)
    {
        if (str[i] == deliminator)
        {
            num_delim++;
        }
    }

    int slen = strlen(str) + 1; // +1 for NULL terminator

    // Allocate memory for tokens
    char** tokens = (char **)malloc(sizeof (char *) * (num_delim + 2)); // +1 for other token, +1 for NULL terminator
    char* mutstr  = (char *)malloc(slen);
    if (!tokens || !mutstr) return NULL;
    
    // Copy and cast values to make gcc happy (:
    const char delim[] = {deliminator, '\0'};
    memcpy(mutstr, str, slen);

    // Split string into tokens
    char* currtok = strtok(mutstr, delim);
    int i = 0;
    while (currtok != NULL)
    {
        tokens[i++] = currtok;
        currtok = strtok(NULL, delim);
    }
    tokens[i] = NULL;
    
    return (const char **)tokens;
}