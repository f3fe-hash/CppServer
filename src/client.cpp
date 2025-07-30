#include "server.hpp"

HTTP* Client::httphandler = nullptr;

Client::Client(int connfd, int clinum, sockaddr_in* addr, HTTP* httphandler)
{
    this->connfd      = connfd;
    this->clinum      = clinum;
    this->addr        = addr;
    this->httphandler = httphandler;
}

Client::~Client()
{}

void Client::handle(ClientArgs* args)
{
    // Read the client's request
    char buff[REQUEST_BUFFER_SIZE];
    ssize_t size = read(args->cli->connfd, buff, REQUEST_BUFFER_SIZE);
    if (size == 0)
        return;
    
    // Parse the request with the HTTP handler
    HTTPRequest* req = args->cli->httphandler->parse_request(buff, size);
    char page[RESPONSE_FILE_PATH_LEN];
    if (strcmp(req->path, "/") == 0)
    {
        strncpy(page, INDEX_FILE, sizeof(page));
        page[RESPONSE_FILE_PATH_LEN - 1] = '\0';  // Ensure null-termination
    }
    else
        snprintf(page, RESPONSE_FILE_PATH_LEN, "site%.507s", req->path);

    // Read file contents
    char filebuff[RESPONSE_FILEBUFF_LEN];
    ssize_t fsize = read_file(page, filebuff, RESPONSE_FILEBUFF_LEN);

    HTTPResponse* res = nullptr;

    if (fsize <= 0)
    {
        static const char* not_found = "<h1>404 Not Found</h1>";
        res = httphandler->generate_response<const char *>(404, not_found, 22);
    }
    else
    {
        filebuff[fsize] = '\0';  // Ensure null termination
        res = httphandler->generate_response<const char *>(200, filebuff, fsize);
    }

    if (!res || !res->data)
    {
        // Use %s to shut up gcc
        server_log(_error_log, "Failed to generate HTTP response");
        FATAL("%s", "Failed to generate HTTP response");
        return;
    }

    size = send(args->cli->connfd, res->data, res->size, 0);
    if (size == 0)
    {
        // Use %s to shut up gcc
        server_log(_error_log, "send failed");
        FATAL("%s", "send failed");
    }
    close(args->cli->connfd);

    args->server->subClient();

    // Deallocate the response
    _alloc->deallocate((void *)res->data);
    _alloc->deallocate((void *)res);

    return;
}