#include <unistd.h>
#include "server.hpp"

int main(int argc, char** argv)
{
    (void)argc;
    (void)argv;
    Server* server = new Server(8080, "192.168.1.28");
    server->clisten();
    usleep(100000000); // 100,000,000 us, or 100 sec
    delete server;
}