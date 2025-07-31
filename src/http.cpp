#include "http.hpp"

// MOST ERROR CODES - NOT ALL OF THEM
//
// I'm NOT implementing 100 error codes, and supporting logic to catch them.
// I already don't support most error codes, nor know how to find most of them,
// nevermind understanding what each does. I'm copy-pasting explanations from
// Some random website.
//
// https://developer.mozilla.org/en-US/docs/Web/HTTP/Reference/Status#informational_responses
const std::unordered_map<int, std::string> errcode_strs =
{
    // 1xx - Information
    {100, "Continue"},
    {101, "Switching Protocols"},
    {102, "Processing"},
    {103, "Early Hints"},

    // 2xx - Successful
    {200, "OK"},
    {201, "Created"},
    {202, "Accepted"},
    {203, "Non-Authoritive Information"},
    {204, "No Content"},

    // 3xx - Redirection
    {301, "Moved Permenantly"},
    {302, "Found"},
    {303, "See Other"},
    {304, "Not Modified"},
    // Error codes 305 and 306 are deprecated
    {307, "Temporary Redirect"},
    {308, "Permanent Redirect"},

    // 4xx - Client error (I am NOT implemeneting error codes up to 451)
    // Also, WTF is 418(I'm a teapot) ?!?!?!
    {400, "Bad Request"},
    {401, "Unauthorized"},
    {403, "Forbidden"},
    {404, "Not Found"},
    {405, "Method Not allowed"},

    // 5xx - Server error
    {500, "Internal Server Error"},
    {501, "Not Implemented"},
    {502, "Bad Gateway"},
    {503, "Service unavailable"},
    {504, "Gateway Timeout"},
    {505, "HTTP Version Not Supported"},
    {506, "Varient Also Negotiates"},
    {507, "Insufficient storage"},
    {508, "Loop Detected"},
    {510, "Not Extended"},
    {511, "Network Authentication Required"}
};

const int MAX_ERROR_CODE_LENGTH = 29;

HTTP::HTTP()
{
    http_calc_regex();
}

HTTP::~HTTP()
{
    http_free_regex();
}

HTTPRequest* HTTP::parse_request(const char* raw, size_t size)
{
    return http_parse_request(raw, size);
}

const char* HTTP::get_content_type(HTTPRequest* req, const char* filename)
{
    (void)req;

    //const char* contentTypes = http_get_header(req, "Accept");
    char* extension = (char *)strstr(filename, ".");

    if (!extension)
        return NULL;
    
    if (strcmp(extension, ".html") == 0)
        return "text/html";
    else if (strcmp(extension, ".css") == 0)
        return "text/css";
    else if (strcmp(extension, ".json") == 0)
        return "application/json";
    
    // Images
    else if (strcmp(extension, ".avif") == 0)
        return "image/avif";
    else if (strcmp(extension, ".webp") == 0)
        return "image/webp";
    else if (strcmp(extension, ".png") == 0)
        return "image/png";
    else if (strcmp(extension, ".svg") == 0)
        return "image/svg";
    else
        return NULL;
}