#include "server.hpp"

Log_t* _info_log  = NULL;
Log_t* _error_log = NULL;

Allocator* _alloc = NULL;

HTTP* Server::httphandler = nullptr;

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
        server_log(_error_log, "Socket bind failed");
        FATAL("Socket bind failed\n");
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
    
    OK("Server UP\n");
    INFO("Server: http://%s:%d\n", ip.c_str(), port);
    printf("%s----------------------------------------------%s\n", COLOR_BLUE, COLOR_RESET);
}

Server::~Server()
{
    this->running = false;

    // Disconnect, and stop allocating threads
    close(this->sockfd);
    
    // Deallocate threads (client threads are already deallocated by the deallocator thread)
    
    // Deallocate logs
    if (_info_log)
    {
        if (_info_log->name) free(_info_log->name);
        free(_info_log);
    }
    if (_error_log)
    {
        if (_error_log->name) free(_error_log->name);
        free(_error_log);
    }

    // Show the cursor again
    printf("\033[?25h");
    
    OK("Stopped the server\n");
}

// Client Listen (To not be confused with sys/socket.h's listen function)
void Server::clisten()
{
    // Start a thread to listen to clients (doesn't count itself as a thread, so the deallocator thread doesn't deallocate it)
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

void Server::_listen(Server* _this)
{
    // Listen to get connections, then create a thread to handle the connection
    while (_this->running == true)
    {
        Client* cli = new (_alloc->allocate(sizeof(Client))) Client();
        if (!cli)
        {
            _this->running = false;
            server_log(_error_log, "Failed to allocate memory for Client structure");
            FATAL("Failed to allocate memory for Client structure");
            _this->running = true;
            continue;
        }
        int len = sizeof(cli->addr);

        // Accept the data packet from client and verification 
        cli->connfd = accept(_this->sockfd, (struct sockaddr *)&cli->addr, (socklen_t *)&len);
        if (cli->connfd < 0)
        {
            _this->running = false;
            server_log(_error_log, "Failed to accept client");
            FATAL("Failed to accept client");
            _alloc->deallocate(cli);
            _this->running = true;
            continue;
        }
        else
        {
            server_log(_info_log, "Accepted new client");
        }

        cli->clinum = _this->num_total_clients;

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
                args->cli = cli;
                args->server = _this;

                // Create Task for thread pool
                Task* task = new_task(reinterpret_cast<void(*)(void *)>(Server::handle_client_wrapper), (void *)args);

                _this->tpool->enqueue(*task);
            }
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
}

void Server::handle_client(ClientArgs* args)
{
    // Alert the server that there is a new client
    INFO("New client (#%d)\r", args->cli->clinum + 1);

    // Create and send the response
    //
    // Use const char* instead of std::string because HTTP::generate_response
    // will serialize the std::string object, which is not good
    HTTPResponse* res = Server::httphandler->generate_response<const char *>(200, MESSAGE);
    if (!res || !res->data)
    {
        args->server->running = false;
        server_log(_error_log, "Failed to generate HTTP response");
        FATAL("Failed to generate HTTP response");
        if (args->cli) _alloc->deallocate((void *)args->cli);
        args->server->running = true;
        return;
    }
    ssize_t sent = send(args->cli->connfd, res->data, res->size, 0);
    if (sent < 0)
    {
        server_log(_error_log, "send failed");
        FATAL("send failed");
    }
    close(args->cli->connfd);

    args->server->num_clients--;

    // Deallocate the response
    _alloc->deallocate((void *)res->data);
    _alloc->deallocate((void *)res);
    _alloc->deallocate((void *)args->cli);
    _alloc->deallocate((void *)args);

    return;
}

void Server::handle_client_wrapper(void* arg)
{
    ClientArgs* args = (ClientArgs*)arg;
    Server::handle_client(args);
}