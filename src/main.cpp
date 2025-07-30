#include <unistd.h>
#include "server.hpp"

int main(int argc, char** argv)
{
    // Make gcc happy  with the -Wextra flag (:
    (void)argc;
    (void)argv;

    Server* server = new Server(8080, "192.168.1.30");
    server->clisten();
    while (true); // 200,000,000 us, or 200 sec

    if (server)
    {
        delete server;
        server = nullptr;
    }
    
    return 0;
}