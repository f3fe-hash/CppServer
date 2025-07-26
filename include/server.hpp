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

#include <csignal>

#include "log.h"
#include "lexer.h"
#include "http.hpp"
#include "memory.hpp"
#include "threads.hpp"

#define MESSAGE "Hello World!"

#define ALLOC_SIZE 128 * MiB

#define DEFAULT_MAX_THREADS 1000 // 1k threads maximum as default
#define DEFAULT_BACKLOG 256

class Server;
class Client;

typedef struct
{
    Client* cli;
    Server* server;
} ClientArgs;

class Client
{
public:
    Client(int connfd, int clinum, sockaddr_in* addr, HTTP* httphandler);
    ~Client();

    static void handle(ClientArgs* args);

    int connfd;
    int clinum;
    struct sockaddr_in* addr;

    static HTTP* httphandler;
};

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

    // Mutexes
    std::mutex clients_mutex;
    std::mutex threads_mutex;

    ThreadPool* tpool;

    static HTTP* httphandler;

    // Multi-threading functions
    static void handle_client(void* args);
    static void _listen(Server* _this);
    static void _climanager(Server* _this);

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

    static Server* serverinst;

    void clisten();
    void stop();
    void start();

    void subClient();

    // Handle Ctrl-C appropriately
    static void sigExit(int signum);
};

#endif