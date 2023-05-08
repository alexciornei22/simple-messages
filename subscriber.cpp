#include <iostream>
#include <cstring>
#include <cstdint>
#include <sstream>
#include <arpa/inet.h>
#include "Socket.hpp"
#include "protocol.hpp"

using namespace std;

int main(int argc, char **argv)
{
    setvbuf(stdout, nullptr, _IONBF, BUFSIZ);

    string id;
    string ip;
    uint16_t port;
    stringstream (argv[1]) >> id;
    stringstream (argv[2]) >> ip;
    stringstream (argv[3]) >> port;

    auto sock = Socket(AF_INET, SOCK_STREAM);

    struct sockaddr_in serv_addr;
    socklen_t socket_len = sizeof(struct sockaddr_in);

    memset(&serv_addr, 0, socket_len);
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(port);
    int rc = inet_pton(AF_INET, ip.data(), &serv_addr.sin_addr.s_addr);
    if (rc < 0)
        throw std::system_error(errno, std::generic_category());

    sock.connect((struct sockaddr *)&serv_addr, socket_len);

    char buf[MAX_MSG_LEN];

    struct msg_hdr msg{TYPE_CONN, htons(sizeof(struct msg_hdr) + id.size() + 1)};
    memcpy(buf, &msg, sizeof(struct msg_hdr));
    memcpy(buf + sizeof(struct msg_hdr), id.data(), id.size() + 1);

    rc = Socket::sendAll(sock.getFd(), buf, ntohs(msg.msg_len));
    if (rc < 0)
        throw std::system_error(errno, std::generic_category());

    while (1) {}
    return 0;
}