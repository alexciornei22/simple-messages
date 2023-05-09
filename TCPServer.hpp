
#ifndef HOMEWORK2_PUBLIC_TCPSERVER_HPP
#define HOMEWORK2_PUBLIC_TCPSERVER_HPP

#include <poll.h>
#include <cstdint>
#include <vector>
#include <list>
#include <string>
#include <unordered_map>
#include <queue>
#include "Socket.hpp"
#include "protocol.hpp"

class Client {
public:
    void setAddr(const sockaddr_in &client_addr);
    void setConnected(bool value);
    void setFd(int fd);
    void setId(const std::string &id);

    std::queue<udp_msg> &getMessageQueue();
    int getFd() const;
    const std::string &getId() const;
    bool isConnected() const;
    const sockaddr_in &getAddr() const;

    void disconnect();
    void connect(int fd, sockaddr_in addr);
    void update(udp_msg msg);
    void sendFromQueue();

private:
    sockaddr_in addr = sockaddr_in();
    bool connected = false;
    int fd;
    std::string id;
    std::queue<udp_msg> message_queue;
};

class Topic {
public:
    void attach(Client *client, bool sf);
    void detach(Client *client);
    void notifyClients(udp_msg msg);
    const std::string &getName() const;
    void setName(const std::string &name);
    const std::list<Client *> &getSfClients() const;
    const std::list<Client *> &getClients() const;

private:
    std::string name;
    std::list<Client *> sf_clients;
    std::list<Client *> clients;
};

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
    std::vector<Client> clients;
    std::vector<Topic> topics;

    void handlePollFds();
    void handleNewConnection();
    void handleConsoleInput();
    void recvUDPTopicData(int fd);
    udp_msg makeUDPMessage(char *buf, sockaddr_in client_addr, int rc);
    void recvTCPClientData(int fd);
    void closeConnection(int fd);

    void handleConnectionMessage(int fd, char *data);
    void handleSubscribeMessage(int fd, const char *data);
    void handleUnSubscribeMessage(int fd, const char *data);
};

#endif //HOMEWORK2_PUBLIC_TCPSERVER_HPP
