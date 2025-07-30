#include "server.hpp"

Log_t* _info_log  = NULL;
Log_t* _error_log = NULL;

Allocator* _alloc = NULL;

HTTP*   Server::httphandler = nullptr;
Server* Server::serverinst  = nullptr;

Server::Server(short port, std::string ip, int max_threads, int backlog)
{
    this->port = port;
    this->ip = ip;
    this->num_clients = 0;
    this->running = true;

    _alloc                  = new Allocator(ALLOC_SIZE);
    this->tpool             = new ThreadPool(max_threads);

    if (Server::httphandler == nullptr)
        Server::httphandler = new HTTP();
    if (Server::serverinst == nullptr)
        Server::serverinst = this;

    open_log(&_info_log, "info.log");
    open_log(&_error_log, "error.log");

    // Hide console cursor
    printf("\e[?25l"); 
  
    // Create socket and verify it is open
    this->sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (this->sockfd == -1)
    {
        server_log(_error_log, "Socket creation failed");
        FATAL("Socket creation failed\n");
        exit(0);
    }
    else
    {
        server_log(_info_log, "Socket successfully created");
        OK("Socket created sucessfully\n");
    }
  
    // Assign ip and port
    this->servaddr.sin_family = AF_INET;
    if (ip == "0.0.0.0" || ip == "INADDR_ANY")
    {
        this->servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    }
    else
    {
        this->servaddr.sin_addr.s_addr = inet_addr(ip.c_str());
    }
    this->servaddr.sin_port = htons(port);

    // Allow address reuse (avoids 'Address already in use' error)
    int opt = 1;
    if (setsockopt(this->sockfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0)
    {
        // Sometimes you won't get the error, so it is a 'note' not 'fatal'
        server_log(_error_log, "Failed to set socket options");
        INFO("Failed to set socket options\n");
    }
    else
    {
        OK("Set socket options\n");
    }
  
    // Binding newly created socket to given IP and verification
    if ((bind(this->sockfd, (struct sockaddr *)&this->servaddr, sizeof(this->servaddr))) != 0)
    {
        // Log the issue
        server_log(_error_log, "Socket bind failed");
        FATAL("Socket bind failed\n");

        // Notify thr user of the most common (If not only) reason for the issue
        INFO("The error might be because of incorrect IP address.\n");
        INFO("Read README.md to see how to fix the issue\n");

        // Can't do anything with incorrect IP address. Exits
        exit(0);
    }
    else
    {
        server_log(_info_log, "Socket successfully bound");
        OK("Socket successfully bound\n");
    }
  
    // Now server is ready to listen and verification
    if (listen(this->sockfd, backlog) != 0)
    {
        server_log(_error_log, "Listen failed");
        FATAL("Listen failed\n");
        exit(0);
    }
    else
    {
        server_log(_info_log, "Server listening");
        OK("Server listening\n");
    }

    std::signal(SIGINT, Server::sigExit);
    
    OK("Server UP\n");
    INFO("Server: http://%s:%d\n", ip.c_str(), port);
    printf("%s----------------------------------------------%s\n", COLOR_BLUE, COLOR_RESET);
}

Server::~Server()
{
    // Disconnect, and stop allocating threads
    close(this->sockfd);
    this->running = false;
    
    // Deallocate logs
    if (_info_log)
    {
        if (_info_log->name)
            free(_info_log->name);
        free(_info_log);
    }

    if (_error_log)
    {
        if (_error_log->name)
            free(_error_log->name);
        free(_error_log);
    }

    delete this->tpool;

    // Show the cursor again
    printf("\033[?25h");
    
    OK("Stopped the server\n");
}

void Server::sigExit(int signum)
{
    // Make gcc happy (:
    (void)signum;

    // Destroy the server
    OK("Terminating...\n");
    serverinst->~Server();
}

// Client Listen (To not be confused with sys/socket.h's listen function by g++)
void Server::clisten()
{
    // Start a thread to listen to clients
    new_thread(reinterpret_cast<void(*)(void *)>(_listen), this);

    // Start a thread to manage client put in the queue
    new_thread(reinterpret_cast<void(*)(void *)>(_climanager), this);
}

void Server::stop()
{
    this->running = false;
    OK("Stopped the server\n");
}

void Server::start()
{
    this->running = true;
    OK("Started the server\n");
}

void Server::subClient()
{
    this->num_clients--;
}

void Server::_listen(Server* _this)
{
    // Listen to get connections, then create a thread to handle the connection
    while (_this->running == true)
    {
        int connfd;
        struct sockaddr_in* addr = (sockaddr_in *)_alloc->allocate(sizeof(sockaddr_in));
        int len = sizeof(addr);

        // Accept the data packet from client and verification 
        connfd = accept(_this->sockfd, (struct sockaddr *)addr, (socklen_t *)&len);
        if (connfd < 0)
        {
            server_log(_error_log, "Failed to accept client");
            FATAL("Failed to accept client");
            _alloc->deallocate(addr);
            continue;
        }
        else
        {
            //server_log(_info_log, "Accepted new client");
        }

        Client* cli = new (_alloc->allocate(sizeof(Client))) Client(connfd, _this->num_total_clients, addr, Server::httphandler);
        if (!cli)
        {
            server_log(_error_log, "Failed to allocate memory for Client structure");
            FATAL("Failed to allocate memory for Client structure");
            continue;
        }

        std::unique_lock<std::mutex> lock(_this->clients_mutex);
        _this->clients.push(cli);

        _this->num_total_clients++;
        _this->num_clients++;
    }
}

void Server::_climanager(Server* _this)
{
    while (_this->running)
    {
        // Lock the queue access if needed
        {
            std::unique_lock<std::mutex> lock(_this->clients_mutex); // Add this mutex to protect queue

            if (!_this->clients.empty())
            {
                Client* cli = _this->clients.front();
                _this->clients.pop();
                lock.unlock();  // Release lock early

                // Allocate task with args using your allocator
                ClientArgs* args = new (_alloc->allocate(sizeof(ClientArgs))) ClientArgs();
                args->cli        = cli;
                args->server     = _this;

                // Create Task for thread pool
                Task* task = new_task(reinterpret_cast<void(*)(void *)>(Server::handle_client), (void *)args);

                _this->tpool->enqueue(*task);

                // Alert the user that there is a new client
                INFO("New client (#%d)\r", _this->num_total_clients + 1);
            }
        }

        // Pause to give the CPU a little break
        //std::this_thread::sleep_for(std::chrono::microseconds(1));
        // No!, INFO is enough of a overhead and IO break to work instead of std::this_thread::sleep_for
    }
}

void Server::handle_client(void* args)
{
    ClientArgs* _args = (ClientArgs *)args;

    _args->cli->handle(_args);

    _alloc->deallocate((void *)_args->cli);
    _alloc->deallocate((void *)_args);
}