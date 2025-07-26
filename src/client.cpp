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
    // Alert the server that there is a new client
    INFO("New client (#%d)\r", args->cli->clinum + 1);

    // Create and send the response
    //
    // Use const char* instead of std::string because HTTP::generate_response
    // will serialize the std::string object, which is not good
    HTTPResponse* res = args->cli->httphandler->generate_response<const char *>(200, MESSAGE);
    if (!res || !res->data)
    {
        server_log(_error_log, "Failed to generate HTTP response");
        FATAL("Failed to generate HTTP response");
        if (args->cli) _alloc->deallocate((void *)args->cli);
        return;
    }

    signed long long int sent = send(args->cli->connfd, res->data, res->size, 0);
    if (sent < 0)
    {
        server_log(_error_log, "send failed");
        FATAL("send failed");
    }
    close(args->cli->connfd);

    args->server->subClient();

    // Deallocate the response
    _alloc->deallocate((void *)res->data);
    _alloc->deallocate((void *)res);
    _alloc->deallocate((void *)args->cli);
    _alloc->deallocate((void *)args);

    return;
}