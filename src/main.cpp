#include <unistd.h>
#include "server.hpp"

int main(int argc, char** argv)
{
    // Make gcc happy (:
    (void)argc;
    (void)argv;

    Server* server = new Server(8080, "192.168.1.30");
    server->clisten();
    usleep(200000000); // 2s00,000,000 us, or 200 sec

    delete server;
    server = nullptr;
    
    return 0;
}