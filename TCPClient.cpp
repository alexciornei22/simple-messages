#include <netinet/tcp.h>
#include <system_error>
#include <arpa/inet.h>
#include <csignal>
#include <cstring>
#include <iostream>
#include <sstream>
#include "TCPClient.hpp"
#include "protocol.hpp"

TCPClient::TCPClient(std::string ip, uint16_t port) {
    server_socket = new Socket(AF_INET, SOCK_STREAM);
    server_socket->enableSockOpt(SOL_SOCKET, SO_REUSEADDR);
    server_socket->enableSockOpt(SOL_TCP, TCP_NODELAY);

    server_addr.sin_family = AF_INET;
    int rc = inet_pton(AF_INET, ip.data(), &server_addr.sin_addr.s_addr);
    if (rc < 0)
        throw std::system_error(errno, std::generic_category(), "inet_pton()");
    server_addr.sin_port = htons(port);
}

TCPClient::~TCPClient() {
    poll_fds.clear();
    delete server_socket;
}

void TCPClient::run(std::string id) {
    server_socket->connect((sockaddr *)&server_addr, sizeof(server_addr));

    poll_fds.push_back(pollfd{STDIN_FILENO, POLLIN, 0});
    poll_fds.push_back(pollfd{server_socket->getFd(), POLLIN, 0});

    char buf[MAX_MSG_LEN];

    msg_hdr msg{TYPE_CONN, 0, htons(sizeof(msg_hdr) + id.size() + 1)};
    memcpy(buf, &msg, sizeof(msg_hdr));
    memcpy(buf + sizeof(msg_hdr), id.data(), id.size() + 1);

    // send ID message
    Socket::sendBuffer(server_socket->getFd(), buf, ntohs(msg.msg_len));

    active = true;
    while (active) {
        int rc = poll(poll_fds.data(), poll_fds.size(), -1);
        if (rc < 0)
            throw std::system_error(errno, std::generic_category(), "poll()");

        for (pollfd pfd: poll_fds) {
            if (pfd.revents & POLLIN) {
                if (pfd.fd == STDIN_FILENO) {
                    getConsoleCommands();
                    continue;
                }
            }
        }
    }
}

void TCPClient::getConsoleCommands() {
    std::string line;
    std::string command;

    getline(std::cin, line);

    std::stringstream s(line);

    s >> command;

    if (command == "exit") {
        active = false;
        return;
    }

    if (command == "subscribe") {
        std::string topic;
        bool sf;

        s >> topic >> sf;
        return;
    }

    if (command == "unsubscribe") {
        std::string topic;

        s >> topic;
        return;
    }
}
