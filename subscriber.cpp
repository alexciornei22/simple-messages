#include <iostream>
#include <sstream>
#include <unistd.h>
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