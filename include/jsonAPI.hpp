#ifndef __JSON_API_HPP__
#define __JSON_API_HPP__

#include <nlohmann/json.hpp>

/*
payload:
{
    "hi": "hi"
}
*/
constexpr const char* default_json =
    "{\n"
    "\t\"hi\": \"hi\"\n"
    "}\0";

class API
{
public:
    API() {};
    virtual ~API() = default;

    virtual char* process_data(void* data, size_t size) = 0;
};

class NullAPI : public API
{
public:
    NullAPI()  : API() {};
    ~NullAPI()         {};

    char* process_data(void* data, size_t size) override
    {
        (void)data; (void)size;

        const size_t len = strlen(default_json);
        char* copy = (char*)malloc(len + 1); // allocate memory
        if (copy)
            memcpy(copy, default_json, len + 1);
        
        return copy; // must be freed by caller
    }
};

#endif