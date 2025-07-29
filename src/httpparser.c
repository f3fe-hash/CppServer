#include "httpparser.h"

// Static regex instances
static regex_t reqline;
static regex_t header;
static int regex_ready = 0;

// Pre-compile regex patterns once
__THROW void http_calc_regex()
{
    static const char* request_line_pattern = "^(GET|POST|PUT|DELETE|HEAD|OPTIONS|PATCH)[ ]+([^ ]+)[ ]+(HTTP/[0-9.]+)";
    static const char* header_pattern = "^([!#$%&'*+.^_`|~0-9A-Za-z-]+):[ ]*(.+)$";

    if (regcomp(&reqline, request_line_pattern, REG_EXTENDED | REG_ICASE) != 0)
    {
        fprintf(stderr, "Failed to compile request line regex\n");
        return;
    }

    if (regcomp(&header, header_pattern, REG_EXTENDED) != 0)
    {
        fprintf(stderr, "Failed to compile header regex\n");
        regfree(&reqline);
        return;
    }

    regex_ready = 1;
}

__THROW __nonnull((1)) HTTPRequest* http_parse_request(const char* raw, ssize_t size)
{
    if (!regex_ready)
        return INVALID_REQUEST;

    regmatch_t matches[4];
    regmatch_t header_matches[3];

    const char* line_end = strstr(raw, "\r\n");
    if (!line_end)
        return INVALID_REQUEST;

    size_t line_len = line_end - raw;
    if (line_len >= 2048)
        return INVALID_REQUEST;

    char line[2048];
    memcpy(line, raw, line_len);
    line[line_len] = '\0';

    if (regexec(&reqline, line, 4, matches, 0) != 0)
        return INVALID_REQUEST;

    HTTPRequest* req = (HTTPRequest*)malloc(sizeof(HTTPRequest));
    if (!req)
        return INVALID_REQUEST;

    memset(req, 0, sizeof(HTTPRequest));

    int len;

    // Method
    len = matches[1].rm_eo - matches[1].rm_so;
    if (len >= HTTP_METHOD_LEN) len = HTTP_METHOD_LEN - 1;
    memcpy(req->method, line + matches[1].rm_so, len);
    req->method[len] = '\0';

    // Path
    len = matches[2].rm_eo - matches[2].rm_so;
    if (len >= HTTP_PATH_LEN) len = HTTP_PATH_LEN - 1;
    memcpy(req->path, line + matches[2].rm_so, len);
    req->path[len] = '\0';

    // Version
    len = matches[3].rm_eo - matches[3].rm_so;
    if (len >= HTTP_VERSION_LEN) len = HTTP_VERSION_LEN - 1;
    memcpy(req->version, line + matches[3].rm_so, len);
    req->version[len] = '\0';

    const char* ptr = line_end + 2;
    req->header_count = 0;

    while (req->header_count < MAX_HTTP_HEADERS)
    {
        const char* next_line = strstr(ptr, "\r\n");
        if (!next_line || next_line == ptr)
        {
            ptr = next_line ? next_line + 2 : ptr;
            break;
        }

        size_t header_len = next_line - ptr;
        if (header_len >= 1024)
        {
            ptr = next_line + 2;
            continue;
        }

        char header_line[1024];
        memcpy(header_line, ptr, header_len);
        header_line[header_len] = '\0';

        if (regexec(&header, header_line, 3, header_matches, 0) == 0)
        {
            int key_len = header_matches[1].rm_eo - header_matches[1].rm_so;
            int val_len = header_matches[2].rm_eo - header_matches[2].rm_so;

            if (key_len < HTTP_HEADER_KEY_LEN && val_len < HTTP_HEADER_VALUE_LEN)
            {
                memcpy(req->headers[req->header_count].key, header_line + header_matches[1].rm_so, key_len);
                req->headers[req->header_count].key[key_len] = '\0';

                memcpy(req->headers[req->header_count].value, header_line + header_matches[2].rm_so, val_len);
                req->headers[req->header_count].value[val_len] = '\0';

                req->header_count++;
            }
        }

        ptr = next_line + 2;
    }

    // Extract body
    ssize_t parsed_offset = ptr - raw;
    if (parsed_offset < size)
    {
        req->payload = raw + parsed_offset;
        req->payload_size = size - parsed_offset;
    }
    else
    {
        req->payload = NULL;
        req->payload_size = 0;
    }

    return req;
}

__THROW __nonnull((1, 2)) const char* http_get_header(HTTPRequest* req, const char* key)
{
    for (int i = 0; i < req->header_count; ++i)
        if (strncmp(req->headers[i].key, key, strlen(key)) == 0)
            return req->headers[i].value;
    return NULL;
}

// Optional cleanup if you want to free the regex later
__THROW void http_free_regex()
{
    if (regex_ready)
    {
        regfree(&reqline);
        regfree(&header);
        regex_ready = 0;
    }
}
