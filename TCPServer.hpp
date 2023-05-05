
#ifndef HOMEWORK2_PUBLIC_TCPSERVER_HPP
#define HOMEWORK2_PUBLIC_TCPSERVER_HPP

#include <poll.h>
#include <cstdint>
#include <vector>
#include <unordered_map>
#include "Socket.hpp"

class TCPServer {
public:
    explicit TCPServer(uint16_t port);
    ~TCPServer();
    void run();

private:
    bool active = false;
    Socket* server_socket;
    struct sockaddr_in serv_addr{};
    socklen_t socket_len{};
    std::vector<pollfd> poll_fds;
    std::unordered_map<int, struct sockaddr_in> address_map;
    void handlePollFds();
    void handleNewConnection();
    void handleConsoleInput();
    void handleIncomingData(int fd);
};

#endif //HOMEWORK2_PUBLIC_TCPSERVER_HPP
