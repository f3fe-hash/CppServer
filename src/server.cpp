#include "server.hpp"

Log_t* _info_log  = NULL;
Log_t* _error_log = NULL;

Server::Server(short port, std::string ip)
{
    this->port = port;
    this->ip = ip;
    this->running = true;
    this->num_clients = 0;

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
    if (listen(this->sockfd, 5) != 0)
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
}

Server::~Server()
{
    this->running = false;

    // Disconnect, and stop allocating threads
    close(this->sockfd);
    if (this->listener->joinable())
        this->listener->join();

    // Wait for all other threads to finish
    for (std::thread* thread : this->threads)
    {
        if (thread->joinable())
            thread->join();
    }

    if (_info_log) {
        if (_info_log->name) free(_info_log->name);
        free(_info_log);
    }
    if (_error_log) {
        if (_error_log->name) free(_error_log->name);
        free(_error_log);
    }
    
    OK("Destoryed the server");
}

// Client Listen (To not be confused with sys/socket.h's listen function)
void Server::clisten()
{
    this->listener = new std::thread(_listen, this);
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
        Client* cli = (Client *)malloc(sizeof(Client));
        if (!cli) {
            server_log(_error_log, "Failed to allocate memory for Client structure");
            FATAL("Failed to allocate memory for Client structure");
            continue;
        }
        int len = sizeof(cli->addr);

        // Accept the data packet from client and verification 
        cli->connfd = accept(_this->sockfd, (struct sockaddr *)&cli->addr, (socklen_t *)&len);
        if (cli->connfd < 0)
        {
            server_log(_error_log, "Failed to accept client");
            FATAL("Failed to accept client");
            free(cli);
            continue;
        }
        else
        {
            server_log(_info_log, "Accepted new client");
        }

        cli->clinum = _this->num_clients;
        std::thread* thread = new std::thread(handle_client, cli);
        _this->threads.push_back(thread);
        _this->num_clients++;
    }
}

void Server::handle_client(Client* cli)
{
    // Alert the server that there is a new client
    INFO("New client %d", cli->clinum);

    // Create and send the response
    Response* res = generate_http_response(MESSAGE);
    if (!res || !res->data) {
        server_log(_error_log, "Failed to generate HTTP response");
        FATAL("Failed to generate HTTP response");
        if (cli) free(cli);
        return;
    }
    ssize_t sent = send(cli->connfd, res->data, res->size, 0);
    if (sent < 0) {
        server_log(_error_log, "send failed");
        FATAL("send failed");
    }
    close(cli->connfd);

    // Deallocate the response
    delete[] res->data;
    delete res;
    free(cli);
}

Response* Server::generate_http_response(const char* msg)
{
    // Calculate the length of the response
    int content_length = strlen(msg);
    int size = 96 + content_length; // Header size (85) + Max decimal places for length of content (10) + message size + Null terminalor (1)

    // Create the response, and set up the values
    Response* res = new Response();
    res->data = new char[size];
    if (!res->data) {
        delete res;
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