#ifndef __LEXER_H__
#define __LEXER_H__

#include <stdlib.h>
#include <string.h>

#ifdef __cplusplus
extern "C"
{
#endif

const char** parse_str(const char* str, const char deliminator) __THROW __nonnull((1));

#ifdef __cplusplus
}
#endif

#endif