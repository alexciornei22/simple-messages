#include "Socket.hpp"
#include <iostream>
#include <cstring>
#include <poll.h>
#include <unistd.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <sys/socket.h>
#include <vector>
#include <system_error>

#include "protocol.hpp"

int Socket::getFd() const {
    return fd;
}

Socket::Socket(int domain, int type) {
    fd = socket(AF_INET, SOCK_STREAM, 0);
    if (fd < 0)
        throw std::system_error(errno, std::generic_category());
}

Socket::~Socket() {
    close(fd);
}

void Socket::enableSockOpt(int level, int optname) const {
    int enable = 1;
    int rc = setsockopt(fd, level, optname, &enable, sizeof(enable));
    if (rc < 0)
        throw std::system_error(errno, std::generic_category());
}

void Socket::bind(const struct sockaddr *addr, socklen_t len) const {
    int rc = ::bind(fd, addr, len);
    if (rc < 0)
        throw std::system_error(errno, std::generic_category());
}

void Socket::listen(int backlog) const {
    int rc = ::listen(fd, backlog);
    if (rc < 0)
        throw std::system_error(errno, std::generic_category());
}

int Socket::accept(struct sockaddr *addr, socklen_t *len) const {
    int new_sockfd = ::accept(fd, addr, len);
    if (new_sockfd < 0)
        throw std::system_error(errno, std::generic_category());

    return new_sockfd;
}

int Socket::recvMessage(int fd, char *buf, size_t *len) {
    int bytes_remaining = sizeof(struct msg_hdr);
    int bytes_received = 0;
    char *aux = buf;

    while (bytes_remaining) {
        int rc = recv(fd, aux, bytes_remaining, 0);
        if (rc < 0)
            throw std::system_error(errno, std::generic_category(), "recv sock");

        bytes_remaining -= rc;
        aux += rc;
        bytes_received += rc;
    }

    struct msg_hdr *hdr = (struct msg_hdr*) buf;
    bytes_remaining = hdr->msg_len - sizeof(struct msg_hdr);
    std::cout << "hdr len " << hdr->msg_len << std::endl;

    while (bytes_remaining > 0) {
        int rc = recv(fd, aux, bytes_remaining, 0);
        if (rc < 0)
            throw std::system_error(errno, std::generic_category(), "recv sock");

        bytes_remaining -= rc;
        aux += rc;
        bytes_received += rc;
        std::cout << "rc " << bytes_remaining << " " << rc << std::endl;
    }

    *len = bytes_received;
    std::cout << bytes_received << std::endl;
    return 0;
}

int Socket::sendAll(int fd, char *buf, size_t len) {
    int bytes_remaining = len;
    char *aux = buf;

    while (bytes_remaining) {
        int rc = send(fd, aux, bytes_remaining, 0);
        if (rc < 0)
            throw std::system_error(errno, std::generic_category());

        bytes_remaining -= rc;
        aux += rc;
        std::cout << "rc " << bytes_remaining << std::endl;
    }

    return send(fd, aux, bytes_remaining, 0);
}

void Socket::connect(struct sockaddr *addr, socklen_t len) {
    int rc = ::connect(fd, addr, len);
    if (rc < 0)
        throw std::system_error(errno, std::generic_category());
}
