#include <iostream>
#include <cstring>
#include <cstdint>
#include <arpa/inet.h>
#include "Socket.hpp"
#include "protocol.hpp"

using namespace std;

int main(int argc, char **argv)
{
    setvbuf(stdout, nullptr, _IONBF, BUFSIZ);

    uint16_t port;
    int rc = sscanf(argv[3], "%hu", &port);
    if (rc < 0)
        throw std::system_error(errno, std::generic_category());

    auto sock = Socket(AF_INET, SOCK_STREAM);

    struct sockaddr_in serv_addr;
    socklen_t socket_len = sizeof(struct sockaddr_in);

    memset(&serv_addr, 0, socket_len);
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(port);
    rc = inet_pton(AF_INET, argv[2], &serv_addr.sin_addr.s_addr);
    if (rc < 0)
        throw std::system_error(errno, std::generic_category());

    sock.connect((struct sockaddr *)&serv_addr, socket_len);

    char buf[MAX_MSG_DATA_LEN];
    size_t len = sizeof(buf);

    struct msg_hdr msg{TYPE_CONN, sizeof(struct msg_hdr) + strlen(argv[1]) + 1};
    memcpy(buf, &msg, sizeof(struct msg_hdr));
    memcpy(buf + sizeof(struct msg_hdr), argv[1], strlen(argv[1]) + 1);

    rc = Socket::sendAll(sock.getFd(), buf, msg.msg_len);
    if (rc < 0)
        throw std::system_error(errno, std::generic_category());

    while (1) {}
    return 0;
}