#include "server.hpp"

Log_t* _info_log  = NULL;
Log_t* _error_log = NULL;

Allocator* _alloc = NULL;

Server::Server(short port, std::string ip, int max_threads, int backlog)
{
    this->port = port;
    this->ip = ip;
    this->num_clients = 0;
    this->running = true;

    _alloc = new Allocator(ALLOC_SIZE);
    this->tpool = new ThreadPool(max_threads);

    open_log(&_info_log, "info.log");
    open_log(&_error_log, "error.log");
  
    // Create socket and verify it is open
    this->sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (this->sockfd == -1)
    {
        server_log(_error_log, "Socket creation failed");
        FATAL("Socket creation failed");
        exit(0);
    }
    else
    {
        server_log(_info_log, "Socket successfully created");
        OK("Socket created sucessfully");
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
        INFO("Failed to set socket options");
    }
    else
    {
        OK("Set socket options");
    }
  
    // Binding newly created socket to given IP and verification
    if ((bind(this->sockfd, (struct sockaddr *)&this->servaddr, sizeof(this->servaddr))) != 0)
    {
        server_log(_error_log, "Socket bind failed");
        FATAL("Socket bind failed");
        exit(0);
    }
    else
    {
        server_log(_info_log, "Socket successfully bound");
        OK("Socket successfully bound");
    }
  
    // Now server is ready to listen and verification
    if (listen(this->sockfd, backlog) != 0)
    {
        server_log(_error_log, "Listen failed");
        FATAL("Listen failed");
        exit(0);
    }
    else
    {
        server_log(_info_log, "Server listening");
        OK("Server listening");
    }
    
    OK("Server UP");
    INFO("Server\t[IP: %s | Port: %d] ( http://%s:%d )", ip.c_str(), port, ip.c_str(), port);
    printf("%s----------------------------------------------------------------------------%s\n", COLOR_BLUE, COLOR_RESET);
}

Server::~Server()
{
    this->running = false;

    // Disconnect, and stop allocating threads
    close(this->sockfd);
    if (this->listener->joinable())
        this->listener->join();
    
    // Dwallocate threads (client threadsa are already deallocated by the deallocator)
    _alloc->deallocate((void *)this->listener);
    
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
    
    OK("Stopped the server");
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
    OK("Stopped the server");
}

void Server::start()
{
    this->running = true;
    OK("Started the server");
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
    INFO("New client (#%d)", args->cli->clinum + 1);

    // Create and send the response
    Response* res = generate_http_response(MESSAGE);
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

Response* Server::generate_http_response(const char* msg)
{
    // Calculate the length of the response
    int content_length = strlen(msg);
    int size = 96 + content_length; // Header size (85) + Max decimal places for length of content (10) + message size + Null terminalor (1)

    // Create the response, and set up the values
    Response* res = new (_alloc->allocate(sizeof(Response))) Response();
    res->data = (char *)_alloc->allocate(size);

    if (!res->data)
    {
        //_alloc->deallocate(res);     Nope! don't deallocate a NULL pointer
        return nullptr;
    }

    res->size = snprintf(res->data, size,
        "HTTP/1.1 200 OK\r\n"
        "Content-Type: text/html\r\n"
        "Content-Length: %d\r\n"
        "Connection: close\r\n"
        "\r\n"
        "%s",
        content_length, msg);
    
    // Return the response
    return res;
}

void Server::handle_client_wrapper(void* arg)
{
    ClientArgs* args = (ClientArgs*)arg;
    Server::handle_client(args);
}