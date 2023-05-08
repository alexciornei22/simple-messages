
#ifndef HOMEWORK2_PUBLIC_TCPSERVER_HPP
#define HOMEWORK2_PUBLIC_TCPSERVER_HPP

#include <poll.h>
#include <cstdint>
#include <vector>
#include <unordered_map>
#include "Socket.hpp"
#include "protocol.hpp"

class TCPServer {
public:
    explicit TCPServer(uint16_t port);
    ~TCPServer();
    void run();

private:
    bool active = false;
    Socket* server_socket;
    Socket* udp_socket;
    sockaddr_in serv_addr = sockaddr_in();
    sockaddr_in udp_addr = sockaddr_in();
    std::vector<pollfd> poll_fds;
    std::unordered_map<int, struct sockaddr_in> address_map;

    void handlePollFds();
    void handleNewConnection();
    void handleConsoleInput();
    udp_msg* recvTopicData(int fd);
    void handleIncomingData(int fd);
    void closeConnection(int fd);
};

#endif //HOMEWORK2_PUBLIC_TCPSERVER_HPP
