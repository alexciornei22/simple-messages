#include <iostream>
#include <cstring>
#include <cstdint>
#include <sstream>
#include <unistd.h>
#include <arpa/inet.h>
#include <vector>
#include <sys/poll.h>
#include "Socket.hpp"
#include "protocol.hpp"
#include "TCPClient.hpp"

using namespace std;

int main(int argc, char **argv)
{
    if (argc < 4) {
        cerr << "Not enough arguments" << endl;
        return 1;
    }
    setvbuf(stdout, nullptr, _IONBF, BUFSIZ);

    string id;
    string ip;
    uint16_t port;
    stringstream (argv[1]) >> id;
    stringstream (argv[2]) >> ip;
    stringstream (argv[3]) >> port;

    TCPClient client = TCPClient(ip, port);
    client.run(id);

    return 0;
}