
#ifndef HOMEWORK2_PUBLIC_SOCKET_HPP
#define HOMEWORK2_PUBLIC_SOCKET_HPP

#include <netinet/in.h>

class Socket {
public:
    Socket(int domain, int type);
    ~Socket();
    void enableSockOpt(int level, int optname) const;
    void bind(const struct sockaddr* addr, socklen_t len) const;
    void listen(int backlog) const;
    int accept(struct sockaddr *addr, socklen_t *len) const;
    static int recvMessage(int fd, char *buf, size_t *len);
    static int sendAll(int fd, char *buf, size_t len);
    void connect(struct sockaddr *addr, socklen_t len);
    [[nodiscard]] int getFd() const;

protected:
    int fd;
};


#endif //HOMEWORK2_PUBLIC_SOCKET_HPP