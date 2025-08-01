#include "server.hpp"

HTTP* Client::httphandler = nullptr;
API*  Client::api         = nullptr;

Client::Client(int connfd, int clinum, sockaddr_in* addr, HTTP* httphandler, API* api)
{
    this->connfd      = connfd;
    this->clinum      = clinum;
    this->addr        = addr;
    this->httphandler = httphandler;
    this->api         = api;
}

Client::~Client()
{
}

void Client::handle(ClientArgs* args)
{
    // Read the client's request
    char buff[REQUEST_BUFFER_SIZE];
    ssize_t size = read(args->cli->connfd, buff, REQUEST_BUFFER_SIZE);
    if (size == 0)
        return;
    
    bool isAPI = false;
    
    // Parse the request with the HTTP handler
    HTTPRequest* req = args->cli->httphandler->parse_request(buff, size);
    if (!req)
    {
        //FATAL("%s", "Failed to parse HTTP request\n");
        close(args->cli->connfd);
        return;
    }

    // Check if user is API
    char page[RESPONSE_FILE_PATH_LEN];
    if (strcmp(req->path, "/") == 0)
    {
        strncpy(page, INDEX_FILE, sizeof(page));
        page[RESPONSE_FILE_PATH_LEN - 1] = '\0';  // Ensure null-termination
    }
    else if (strcmp(req->path, "/api") == 0)
    {
        isAPI = true;
    }
    else
        snprintf(page, RESPONSE_FILE_PATH_LEN, "site%.507s", req->path);

    HTTPResponse* res = nullptr;
    if (!isAPI) // Isn't API. Handle a human interaction, and send a file
    {
        // Read file contents
        char filebuff[RESPONSE_FILEBUFF_LEN];
        ssize_t fsize = read_file(page, filebuff, RESPONSE_FILEBUFF_LEN);

        char* contentType = (char *)httphandler->get_content_type(req, page);

        if ((fsize <= 0) || !contentType)
        {
            // 404.html
            printf("404");
            static char filebuff[RESPONSE_FILEBUFF_LEN];
            ssize_t fsize = read_file("site/404.html", filebuff, RESPONSE_FILEBUFF_LEN);
            filebuff[fsize] = '\0';  // Ensure null termination
            res = httphandler->generate_response<const char *>(404, (char *)"text/html", filebuff, fsize);
        }
        else
        {
            filebuff[fsize] = '\0';  // Ensure null termination
            res = httphandler->generate_response<const char *>(200, contentType, filebuff, fsize);
        }
    }
    // API, invoke the API class to use the API
    else
    {
        char* data = (char *)args->cli->api->process_data(buff, REQUEST_BUFFER_SIZE);
        res = httphandler->generate_response<const char *>(
            200,
            (char *)"application/json",
            data,
            strlen(data));
        send(args->cli->connfd, res->data, res->size, 0);
        free(data);
    }

    if (!res || !res->data)
    {
        // Use %s to shut up gcc
        FATAL("%s", "Failed to generate HTTP response");
        return;
    }

    size = send(args->cli->connfd, res->data, res->size, 0);
    if (size == 0)
    {
        // Use %s to shut up gcc
        FATAL("%s", "send failed\n");
    }
    close(args->cli->connfd);

    args->server->subClient();

    // Deallocate the response
    _alloc->deallocate((void *)res->data);
    _alloc->deallocate((void *)res);

    return;
}