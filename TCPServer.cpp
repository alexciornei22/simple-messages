#include "TCPServer.hpp"
#include "protocol.hpp"

#include <cstring>
#include <iostream>
#include <system_error>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/tcp.h>

TCPServer::TCPServer(uint16_t port) : server_socket(new Socket(AF_INET, SOCK_STREAM)) {
    server_socket->enableSockOpt(SOL_SOCKET, SO_REUSEADDR);
    server_socket->enableSockOpt(SOL_TCP, TCP_NODELAY);

    socket_len = sizeof(struct sockaddr_in);
    memset(&serv_addr, 0, socket_len);
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    serv_addr.sin_port = htons(port);

    server_socket->bind((const struct sockaddr *)&serv_addr, socket_len);
}

void TCPServer::run() {
    server_socket->listen(10);

    poll_fds.push_back(pollfd{server_socket->getFd(), POLLIN, 0});
    poll_fds.push_back(pollfd{STDIN_FILENO, POLLIN, 0});

    active = true;
    while (active) {
        int rc = poll(poll_fds.data(), poll_fds.size(), -1);
        if (rc < 0)
            throw std::system_error(errno, std::generic_category(), "poll");

        handlePollFds();
    }
}

void TCPServer::handlePollFds() {
    /*
     * using normal index loop instead of range-based loop
     * because the vector may be resized during the loop,
     * making all iterators invalid resulting in undefined behaviour
     */
    for (int i = 0; i < poll_fds.size(); i++) {
        pollfd pfd = poll_fds[i];

        if (pfd.revents & POLLIN) {
            if (pfd.fd == server_socket->getFd()) {
                handleNewConnection();
                break;
            }

            if (pfd.fd == STDIN_FILENO) {
                handleConsoleInput();
                break;
            }

                handleIncomingData(pfd.fd);
            break;
        }
    }
}

void TCPServer::handleNewConnection() {
    struct sockaddr_in client_addr{};
    socklen_t client_len = sizeof(client_addr);

    int new_sockfd = server_socket->accept((struct sockaddr *)&client_addr, &client_len);

    poll_fds.push_back(pollfd{new_sockfd, POLLIN, 0});
}

void TCPServer::handleConsoleInput() {
    std::string command;

    getline(std::cin, command);

    if (command == "exit") {
        active = false;
    }
}

void TCPServer::handleIncomingData(int fd) {
    char buf[MAX_MSG_DATA_LEN];
    size_t len;

    int rc = Socket::recvMessage(fd, buf, &len);
    if (rc < 0)
        throw std::system_error(errno, std::generic_category(), "recvall");

    struct msg_hdr *msg = (struct msg_hdr*) buf;

    if (msg->type == TYPE_CONN) {
        char id[ID_MAX_LEN];
        memcpy(id, buf + sizeof(struct msg_hdr), ID_MAX_LEN);

        std::cout << "New client " << id << " connected from IP:PORT." << std::endl;
    }
}

TCPServer::~TCPServer() {
    poll_fds.clear();
    delete server_socket;
}
