#include <iostream>
#include <cstring>
#include <poll.h>
#include <unistd.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <sys/socket.h>
#include <vector>
#include <sstream>
#include "TCPServer.hpp"

using namespace std;

int main(int argc, char **argv)
{
    setvbuf(stdout, nullptr, _IONBF, BUFSIZ);

    uint16_t port;
    stringstream (argv[1]) >> port;

    TCPServer server = TCPServer(port);
    server.run();
    return 0;
}
