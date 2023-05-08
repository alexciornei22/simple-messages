#include "TCPServer.hpp"
#include "protocol.hpp"

#include <cstring>
#include <iostream>
#include <algorithm>
#include <exception>
#include <system_error>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>

class connection_ended : public std::exception {
    [[nodiscard]] const char * what() const noexcept override {
        return "Connection Ended";
    }
};

TCPServer::TCPServer(uint16_t port) {
    server_socket = new Socket(AF_INET, SOCK_STREAM);
    server_socket->enableSockOpt(SOL_SOCKET, SO_REUSEADDR);
    server_socket->enableSockOpt(SOL_TCP, TCP_NODELAY);

    udp_socket = new Socket(AF_INET, SOCK_DGRAM);
    udp_socket->enableSockOpt(SOL_SOCKET, SO_REUSEADDR);

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    serv_addr.sin_port = htons(port);

    udp_addr.sin_family = AF_INET;
    udp_addr.sin_addr.s_addr = INADDR_ANY;
    udp_addr.sin_port = htons(port);

    server_socket->bind((const struct sockaddr *)&serv_addr, sizeof(serv_addr));
    udp_socket->bind((const struct sockaddr *)&udp_addr, sizeof(udp_addr));
}

void TCPServer::run() {
    server_socket->listen(10);

    poll_fds.push_back(pollfd{server_socket->getFd(), POLLIN, 0});
    poll_fds.push_back(pollfd{udp_socket->getFd(), POLLIN, 0});
    poll_fds.push_back(pollfd{STDIN_FILENO, POLLIN, 0});

    active = true;
    while (active) {
        int rc = poll(poll_fds.data(), poll_fds.size(), -1);
        if (rc < 0)
            throw std::system_error(errno, std::generic_category(), "poll()");

        handlePollFds();
    }
}

void TCPServer::handlePollFds() {
    /*
     * using normal index loop instead of range-based loop
     * because the vector may be resized during the loop,
     * making all iterators invalid resulting in undefined behaviour
     */
    for (size_t i = 0; i < poll_fds.size(); i++) {
        pollfd pfd = poll_fds[i];

        if (pfd.revents & POLLIN) {
            if (pfd.fd == server_socket->getFd()) {
                handleNewConnection();
                continue;
            }

            if (pfd.fd == udp_socket->getFd()) {
                recvTopicData(pfd.fd);
                continue;
            }

            if (pfd.fd == STDIN_FILENO) {
                handleConsoleInput();
                continue;
            }

            try {
                handleIncomingData(pfd.fd);
            } catch (connection_ended &e) {
                closeConnection(pfd.fd);
            }
        }
    }
}

void TCPServer::handleNewConnection() {
    struct sockaddr_in client_addr{};
    socklen_t client_len = sizeof(client_addr);

    int new_sockfd = server_socket->accept((struct sockaddr *)&client_addr, &client_len);

    poll_fds.push_back(pollfd{new_sockfd, POLLIN, 0});
    address_map.insert({new_sockfd, client_addr});
}

void TCPServer::handleConsoleInput() {
    std::string command;

    getline(std::cin, command);

    if (command == "exit") {
        active = false;
    }
}

void TCPServer::handleIncomingData(int fd) {
    char buf[MAX_MSG_LEN];
    size_t len;

    int ret = Socket::recvMessage(fd, buf, &len);
    if (ret == 0) {
        throw connection_ended();
    }

    auto *msg = (struct msg_hdr*) buf;

    if (msg->type == TYPE_CONN) {
        char id[ID_MAX_LEN];
        struct sockaddr_in client_addr = address_map.at(fd);
        memcpy(id, buf + sizeof(struct msg_hdr), ID_MAX_LEN);

        std::cout << "New client " << id << " connected from " <<
        inet_ntoa(client_addr.sin_addr) << ":" << client_addr.sin_port <<
        std::endl;
    }
}

TCPServer::~TCPServer() {
    poll_fds.clear();
    address_map.clear();
    delete server_socket;
    delete udp_socket;
}

void TCPServer::closeConnection(int fd) {
    poll_fds.erase(std::remove_if(poll_fds.begin(), poll_fds.end(), [&](const pollfd &item) {
        return item.fd == fd;
    }),poll_fds.end());

    address_map.erase(fd);
    int rc = close(fd);
    if (rc < 0)
        throw std::system_error(errno, std::generic_category(), "close()");

    std::cout << "Connection ended" << std::endl;
}

void TCPServer::recvTopicData(int fd) {
    sockaddr_in client_addr = sockaddr_in();
    socklen_t client_len = sizeof(client_addr);
    char buf[MAX_MSG_LEN] = {0};

    ssize_t rc = recvfrom(fd, buf, MAX_MSG_LEN, 0, (struct sockaddr *)&client_addr, &client_len);
    if (rc < 0)
        throw std::system_error(errno, std::generic_category(), "recvfrom()");

    udp_msg *new_msg = getMessage(buf, client_addr);
}

udp_msg *TCPServer::getMessage(char *buf, sockaddr_in client_addr) {
    auto *msg = new udp_msg();
    msg->client_addr = client_addr.sin_addr.s_addr;
    msg->client_port = client_addr.sin_port;

    char *aux = buf;
    snprintf(msg->topic, TOPIC_MAX_LEN, "%s", aux);
    aux += TOPIC_MAX_LEN;
    msg->type = (uint8_t) *aux;
    aux++;
    memcpy(msg->data, aux, DATA_MAX_LEN);

    return msg;
}
