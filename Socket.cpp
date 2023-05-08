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
    fd = socket(domain, type, 0);
    if (fd < 0)
        throw std::system_error(errno, std::generic_category(), "socket()");
}

Socket::~Socket() {
    close(fd);
}

void Socket::enableSockOpt(int level, int optname) const {
    int enable = 1;
    int rc = setsockopt(fd, level, optname, &enable, sizeof(enable));
    if (rc < 0)
        throw std::system_error(errno, std::generic_category(), "setsockopt()");
}

void Socket::bind(const struct sockaddr *addr, socklen_t len) const {
    int rc = ::bind(fd, addr, len);
    if (rc < 0)
        throw std::system_error(errno, std::generic_category(), "bind()");
}

void Socket::listen(int backlog) const {
    int rc = ::listen(fd, backlog);
    if (rc < 0)
        throw std::system_error(errno, std::generic_category(), "listen()");
}

int Socket::accept(struct sockaddr *addr, socklen_t *len) const {
    int new_sockfd = ::accept(fd, addr, len);
    if (new_sockfd < 0)
        throw std::system_error(errno, std::generic_category());

    return new_sockfd;
}

int Socket::recvMessage(int fd, char *buf, size_t *len) {
    int rc = recvNBytes(fd, buf, sizeof(msg_hdr));
    if (rc == 0) // connection ended
        return 0;

    auto *hdr = (struct msg_hdr*) buf;
    hdr->msg_len = ntohs(hdr->msg_len);
    size_t data_bytes = hdr->msg_len - sizeof(msg_hdr);
    std::cout << "data len " << data_bytes << std::endl;

    rc = recvNBytes(fd, buf + sizeof(msg_hdr), data_bytes);
    if (rc == 0) // connection ended
        return 0;

    *len = hdr->msg_len;
    return hdr->msg_len;
}

void Socket::sendBuffer(int fd, char *buf, size_t len) {
    int bytes_remaining = len;
    char *aux = buf;

    while (bytes_remaining) {
        int rc = send(fd, aux, bytes_remaining, 0);
        if (rc < 0)
            throw std::system_error(errno, std::generic_category(), "send()");

        bytes_remaining -= rc;
        aux += rc;
    }
}

void Socket::connect(struct sockaddr *addr, socklen_t len) const {
    int rc = ::connect(fd, addr, len);
    if (rc < 0)
        throw std::system_error(errno, std::generic_category(), "connect()");
}

int Socket::recvNBytes(int fd, char *buf, size_t n) {
    int bytes_remaining = n;
    int bytes_received = 0;
    char *aux = buf;

    while (bytes_remaining) {
        int rc = recv(fd, aux, bytes_remaining, 0);
        if (rc < 0)
            throw std::system_error(errno, std::generic_category(), "recv sock");

        if (rc == 0) // connection ended
            return 0;

        bytes_remaining -= rc;
        bytes_received += rc;
        aux += rc;
    }
    return bytes_received;
}
