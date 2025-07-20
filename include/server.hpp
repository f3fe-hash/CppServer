#ifndef __SERVER_HPP__
#define __SERVER_HPP__

#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <unistd.h>

#include <string>
#include <iostream>

#include <vector>
#include <queue>
#include <unordered_map>

#include <thread>
#include <mutex>

#include <cmath>

#include "log.h"
#include "lexer.h"
#include "http.hpp"
#include "memory.hpp"
#include "threads.hpp"

#define MESSAGE "Hello World!"

#define ALLOC_SIZE 64 * MiB

#define DEFAULT_MAX_THREADS 500 // 500 threads maximum as default
#define DEFAULT_BACKLOG 128

class Server;

typedef struct
{
    int connfd;
    int clinum;
    struct sockaddr_in addr;
} Client;

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

    int num_clients;        // Number of clients currently using the server
    int num_total_clients;  // Total clients the server handled since it was declared UP

    struct sockaddr_in servaddr;

    std::queue<Client *> clients; // Client queue
    std::queue<std::thread *> threads; // Queue of threads. Checked by deallocator to remove done threads

    std::mutex clients_mutex;
    std::mutex threads_mutex;

    // Specific-job threads

    ThreadPool* tpool;
    static HTTP* httphandler;

    // Multi-threading functions
    static void handle_client(ClientArgs* client);
    static void _listen(Server* _this);
    static void _climanager(Server* _this);

    static void handle_client_wrapper(void* arg);

    inline void new_thread(void (*func)(void *), void* args)
    {
        Task* task = new (_alloc->allocate(sizeof(Task))) Task;
        task->task = func;
        task->args = args;

        this->tpool->enqueue(*task);
    }

public:
    Server(short port, std::string ip, int max_threads=DEFAULT_MAX_THREADS, int backlog=DEFAULT_BACKLOG);
    ~Server();

    void clisten();
    void stop();
    void start();
};

#endif