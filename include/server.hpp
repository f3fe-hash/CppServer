#ifndef __SERVER_HPP__
#define __SERVER_HPP__

#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <unistd.h>

#include <string>
#include <vector>
#include <thread>
#include <iostream>

#include <cmath>

#include "log.h"
#include "memory.hpp"

#define MESSAGE "Hello World!"

#define ALLOC_SIZE 2 * MiB

class Server;

typedef struct
{
    int connfd;
    int clinum;
    struct sockaddr_in addr;
} Client;

typedef struct
{
    char* data;
    ssize_t size;
} Response;

typedef struct
{
    Server* server;
    Client* cli;
} ClientArgs;

extern Log_t* _info_log;
extern Log_t* _error_log;

extern Allocator* _alloc;

class Server
{
    short port;
    std::string ip;

    int sockfd;
    int running;

    int num_clients;

    struct sockaddr_in servaddr;

    std::vector<Client *> clients;
    std::vector<std::thread *> threads;
    std::thread* listener;

    static void handle_client(ClientArgs* client);
    static void _listen(Server* _this);

    static Response* generate_http_response(const char* msg);
public:
    Server(short port, std::string ip);
    ~Server();

    void clisten();
    void stop();
    void start();
};

#endif