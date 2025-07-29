#ifndef __HTTP_H__
#define __HTTP_H__

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <regex.h>
#include <sys/types.h>

#define HTTP_METHOD_LEN       8
#define HTTP_PATH_LEN         1024
#define HTTP_VERSION_LEN      16
#define HTTP_CONTENT_TYPE_LEN 32
#define HTTP_HEADER_KEY_LEN   64
#define HTTP_HEADER_VALUE_LEN 256
#define MAX_HTTP_HEADERS      32

#ifdef __cplusplus
extern "C"
{
#endif

typedef struct
{
    char key[HTTP_HEADER_KEY_LEN];
    char value[HTTP_HEADER_VALUE_LEN];
} HTTPHeader;

typedef struct
{
    char method[HTTP_METHOD_LEN];
    char path[HTTP_PATH_LEN];
    char version[HTTP_VERSION_LEN];
    char contentType[HTTP_CONTENT_TYPE_LEN]; // Optional field, unused in parser but defined
    HTTPHeader headers[MAX_HTTP_HEADERS];
    int header_count;

    const char* payload;    // Points into original request buffer
    ssize_t payload_size;
} HTTPRequest;

extern regex_t* header_regex, reqline_regex;

#define INVALID_REQUEST ((HTTPRequest *)0x00)

void http_calc_regex() __THROW;

// Parse and return request (NULL on failure)
HTTPRequest* http_parse_request(const char* raw, ssize_t size) __THROW __nonnull((1));

// Utility: Get header value by key (NULL if not found)
const char* http_get_header(HTTPRequest* req, const char* key) __THROW __nonnull((1, 2));

void http_free_regex();

#ifdef __cplusplus
}
#endif

#endif
