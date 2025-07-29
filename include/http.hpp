#ifndef __HTTP_HPP__
#define __HTTP_HPP__

#include <string>
#include <istream>
#include <unordered_map>

#include <string.h>
#include <regex.h>

#include "httpparser.h"

#include "memory.hpp"

extern Allocator* _alloc;
extern const std::unordered_map<int, std::string> errcode_strs;
extern const int  MAX_ERROR_CODE_LENGTH;

typedef struct
{
    char* data;
    ssize_t size;
} HTTPResponse;

class HTTP
{
public:
    HTTP();
    ~HTTP();

    static HTTPRequest* parse_request(const char* raw, size_t size);

    template <typename T>
    static HTTPResponse* generate_response(int error_code, T data, size_t datasize) __THROW
    {
        const void* _data = static_cast<const void *>(data);

        // Calculate the length of the response
        int size = 94 + MAX_ERROR_CODE_LENGTH + datasize; // Header size (83) + Maximum error code length () + Max decimal places for length of content (10) + data size + Null terminalor (1)

        // Create the response, and set up the values
        HTTPResponse* res = new (_alloc->allocate(sizeof(HTTPResponse))) HTTPResponse;
        res->data = (char *)_alloc->allocate(size);

        if (!res->data)
        {
            //_alloc->deallocate(res);     Nope! don't deallocate a NULL pointer
            return nullptr;
        }

        std::string errcode_str = errcode_strs.at(error_code);
        const char* str = reinterpret_cast<const char*>(_data);

        res->size = snprintf(res->data, size,
            "HTTP/1.1 %d %s\r\n"
            "Content-Type: text/html\r\n"
            "Content-Length: %ld\r\n"
            "Connection: close\r\n"
            "\r\n"
            "%s",
            error_code, errcode_str.c_str(),
            datasize, str);
    
        // Return the response
        return res;
    }
};

#endif