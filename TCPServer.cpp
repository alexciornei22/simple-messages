#include "TCPServer.hpp"
#include "protocol.hpp"

#include <cstring>
#include <iostream>
#include <sstream>
#include <algorithm>
#include <exception>
#include <system_error>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/tcp.h>
#include <netinet/in.h>
#include <arpa/inet.h>

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

    clients.reserve(100);
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

//        std::cout << topics.size() << " " << clients.size() << std::endl;
//        std::cout << "--------------------\n";
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
                recvUDPTopicData(pfd.fd);
                continue;
            }

            if (pfd.fd == STDIN_FILENO) {
                handleConsoleInput();
                continue;
            }

            try {
                recvTCPClientData(pfd.fd);
            } catch (connection_ended &e) {
                closeConnection(pfd.fd);
            }
        }
    }
}

void TCPServer::handleNewConnection() {
    sockaddr_in client_addr = sockaddr_in();
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

void TCPServer::recvTCPClientData(int fd) {
    char buf[MAX_MSG_LEN];
    size_t len;

    int ret = Socket::recvMessage(fd, buf, &len);
    if (ret == 0) {
        throw connection_ended();
    }

    auto *msg = (msg_hdr*) buf;
    char *data = buf + sizeof(msg_hdr);

    switch (msg->type) {
        case TYPE_CONN:
            handleConnectionMessage(fd, data);
            break;
        case TYPE_SUB:
            handleSubscribeMessage(fd, data);
            break;
        case TYPE_UNSUB:
            handleUnSubscribeMessage(fd, data);
            break;
    }

}

TCPServer::~TCPServer() {
    poll_fds.clear();
    topics.clear();
    clients.clear();
    delete server_socket;
    delete udp_socket;
}

void TCPServer::closeConnection(int fd) {
    poll_fds.erase(std::remove_if(poll_fds.begin(), poll_fds.end(), [&](const pollfd &item) {
        return item.fd == fd;
    }),poll_fds.end());

    int rc = close(fd);
    if (rc < 0)
        throw std::system_error(errno, std::generic_category(), "close()");

    // find client by FD
    auto client = std::find_if(clients.begin(), clients.end(), [&](const Client &item) {
        return item.getFd() == fd;
    });

    if (client != clients.end()) {
        client->disconnect();
    }
}

void TCPServer::recvUDPTopicData(int fd) {
    sockaddr_in client_addr = sockaddr_in();
    socklen_t client_len = sizeof(client_addr);
    char buf[MAX_MSG_LEN]{};

    ssize_t rc = recvfrom(fd, buf, MAX_MSG_LEN, 0, (struct sockaddr *)&client_addr, &client_len);
    if (rc < 0)
        throw std::system_error(errno, std::generic_category(), "recvfrom()");

    udp_msg new_msg = makeUDPMessage(buf, client_addr, rc);

//    std::cout << "recvudp " << new_msg.topic << " " << +new_msg.type << "\n";

    std::string topic_name = new_msg.topic;
//    std::cout << topic_name << "\n";
    // find topic by name
    auto topic = std::find_if(topics.begin(), topics.end(), [&](const Topic &item) {
        return item.getName() == new_msg.topic;
    });

    if (topic != topics.end()) {
        topic->notifyClients(new_msg);
    }
}

udp_msg TCPServer::makeUDPMessage(char *buf, sockaddr_in client_addr, int rc) {
    auto msg = udp_msg();

    msg.client_addr = htonl(client_addr.sin_addr.s_addr);
    msg.client_port = htons(client_addr.sin_port);
    msg.size = htons(sizeof(udp_msg) - sizeof(msg.data) - sizeof(msg.topic) - sizeof(msg.type) + rc);
//    std::cout << "make: " << rc << " " << sizeof(msg.data) << " " << sizeof(udp_msg) << "\n";

    char *aux = buf;
    snprintf(msg.topic, TOPIC_MAX_LEN, "%s", aux);
    aux += TOPIC_MAX_LEN - 1;
    msg.type = (uint8_t) *aux;
    aux++;
    memcpy(msg.data, aux, DATA_MAX_LEN);

    return msg;
}

void TCPServer::handleConnectionMessage(int fd, char *data) {
    char id[ID_MAX_LEN];
    memcpy(id, data, ID_MAX_LEN);

    // find client by ID
    auto client = std::find_if(clients.begin(), clients.end(), [&](const Client &item) {
        return item.getId() == id;
    });

    sockaddr_in client_addr = sockaddr_in();
    socklen_t client_len = sizeof(client_addr);
    int rc = getpeername(fd, (sockaddr*)&client_addr, &client_len);
    if (rc < 0)
        throw std::system_error(errno, std::generic_category(), "getpeername()");

    if (client == clients.end()) { // new client
        Client new_client = Client();
        new_client.setFd(fd);
        new_client.setId(id);
        new_client.setAddr(client_addr);
        new_client.setConnected(true);

        clients.push_back(new_client);

        std::cout << "New client " << id << " connected from " <<
                  inet_ntoa(client_addr.sin_addr) << ":" << client_addr.sin_port <<
                  std::endl;
    } else {
        if (client->isConnected()) { // client already connected
            std::cout << "Client " << client->getId() << " already connected." << std::endl;

            closeConnection(fd);
        } else { // old client reconnects
            client->connect(fd, client_addr);

            std::cout << "New client " << id << " connected from " <<
                      inet_ntoa(client_addr.sin_addr) << ":" << client_addr.sin_port <<
                      std::endl;
        }
    }
}

void TCPServer::handleSubscribeMessage(int fd, const char *data) {
    auto *msg = (sub_msg *) data;
//    std::cout << msg->topic_name << "\n";
//    std::cout << +msg->sf << " " << msg->topic_name << "\n";

    // find client by FD
    auto client = std::find_if(clients.begin(), clients.end(), [&](const Client &item) {
        return item.getFd() == fd;
    });

    // find topic by name
    auto topic = std::find_if(topics.begin(), topics.end(), [&](const Topic &item) {
        return item.getName() == msg->topic_name;
    });

    if (topic == topics.end()) {
        auto new_topic = Topic();
        new_topic.setName(msg->topic_name);
        new_topic.attach(&(*client), msg->sf);

        topics.push_back(new_topic);
    } else {
        topic->attach(&(*client), msg->sf);
    }
}

void TCPServer::handleUnSubscribeMessage(int fd, const char *data) {
    auto *msg = (unsub_msg *) data;
//    std::cout << msg->topic_name << "\n";
//    std::cout << +msg->sf << " " << msg->topic_name << "\n";

    // find client by FD
    auto client = std::find_if(clients.begin(), clients.end(), [&](const Client &item) {
        return item.getFd() == fd;
    });

    // find topic by name
    auto topic = std::find_if(topics.begin(), topics.end(), [&](const Topic &item) {
        return item.getName() == msg->topic_name;
    });

    if (topic != topics.end()) {
        topic->detach(&(*client));
    }
}


void Client::setAddr(const sockaddr_in &client_addr) {
    Client::addr = client_addr;
}

void Client::setConnected(bool value) {
    Client::connected = value;
}

std::queue<udp_msg> &Client::getMessageQueue() {
    return message_queue;
}

int Client::getFd() const {
    return fd;
}

void Client::setFd(int fd) {
    Client::fd = fd;
}

const std::string &Client::getId() const {
    return id;
}

void Client::setId(const std::string &id) {
    Client::id = id;
}

bool Client::isConnected() const {
    return connected;
}

const sockaddr_in &Client::getAddr() const {
    return addr;
}

void Client::disconnect() {
    connected = false;
    fd = -1;

    std::cout << "Client " << this->getId() << " disconnected." << std::endl;
}

void Client::connect(int fd, sockaddr_in addr) {
    this->setFd(fd);
    this->setAddr(addr);
    this->setConnected(true);

    this->sendFromQueue();
}

void Client::update(udp_msg msg) {
    char buf[MAX_MSG_LEN];
    size_t buf_len = 0;

//    std::cout << "update " << msg.topic << " " << msg.type << " " << msg.data << "\n";

    buf_len += sizeof(msg_hdr);
    buf_len += ntohs(msg.size);

    auto *hdr = (msg_hdr *)buf;
    hdr->type = TYPE_TOPDAT;
    hdr->msg_len = htons(buf_len);

//    std::cout << "update: " << buf_len << " " << ntohs(msg.size) << std::endl;
//    std::cout << msg.topic << " " << +msg.type << " " << msg.data << "\n";
    memcpy(buf + sizeof(msg_hdr), &msg, ntohs(msg.size));

    Socket::sendBuffer(this->getFd(), buf, buf_len);
}

void Client::sendFromQueue() {
    while (!message_queue.empty()) {
        auto msg = message_queue.front();
        message_queue.pop();

        this->update(msg);
    }
}

void Topic::attach(Client *client, bool sf) {
    this->detach(client);

    if (sf) {
        sf_clients.push_back(client);
    } else {
        clients.push_back(client);
    }
//    std::cout << name << ":" << sf_clients.size() + clients.size() << std::endl;
}

void Topic::detach(Client *client) {
    sf_clients.remove(client);
    clients.remove(client);
}

const std::string &Topic::getName() const {
    return name;
}

void Topic::setName(const std::string &name) {
    Topic::name = name;
}

const std::list<Client *> &Topic::getSfClients() const {
    return sf_clients;
}

const std::list<Client *> &Topic::getClients() const {
    return clients;
}

void Topic::notifyClients(udp_msg msg) {
    for (auto client : clients) {
        if (client->isConnected()) {
            client->update(msg);
        }
    }

    for (auto client : sf_clients) {
        if (client->isConnected()) {
            client->update(msg);
        } else {
            client->getMessageQueue().push(msg);
        }
    }
}
