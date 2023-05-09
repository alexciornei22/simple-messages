#include <netinet/tcp.h>
#include <system_error>
#include <arpa/inet.h>
#include <csignal>
#include <cstring>
#include <iostream>
#include <iomanip>
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

    msg_hdr msg{TYPE_CONN, htons(sizeof(msg_hdr) + id.size() + 1)};
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

                if (pfd.fd == server_socket->getFd()) {
                    try {
                        recvData();
                    } catch (connection_ended &e) {
                        active = false;
                    }
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
        sendSubscribeMessage(topic, sf);
        return;
    }

    if (command == "unsubscribe") {
        std::string topic;

        s >> topic;
        sendUnSubscribeMessage(topic);
        return;
    }
}

void TCPClient::sendSubscribeMessage(std::string topic, bool sf) {
    char buf[MAX_MSG_LEN];
    uint16_t buf_len = 0;

    sub_msg data = sub_msg();
    data.sf = sf;
    memcpy(data.topic_name, topic.c_str(), TOPIC_MAX_LEN);

//    std::cout << +data.sf << " " << data.topic_name << std::endl;

    buf_len += sizeof(msg_hdr);
    buf_len += sizeof(uint8_t);
    buf_len += topic.size() + 1;

    msg_hdr msg{TYPE_SUB, htons(buf_len)};

    memcpy(buf, &msg, sizeof(msg_hdr));
    memcpy(buf + sizeof(msg_hdr), &data, sizeof(data));

    Socket::sendBuffer(server_socket->getFd(), buf, buf_len);

    std::cout << "Subscribed to topic." << std::endl;
}

void TCPClient::recvData() {
    char buf[MAX_MSG_LEN];
    size_t buf_len;

    int rc = Socket::recvMessage(server_socket->getFd(), buf, &buf_len);
    if (rc == 0)
        throw connection_ended();

    auto msg = (udp_msg *) (buf + sizeof(msg_hdr));
    auto data = msg->data;

//    std::cout << msg->topic << " " << msg->type << " " << msg->data << "\n";

    in_addr addr = in_addr();
    addr.s_addr = ntohl(msg->client_addr);
    in_port_t port = ntohs(msg->client_port);

    // print udp publisher address
    std::cout << inet_ntoa(addr) << ":" << port << " - ";
    std::cout << msg->topic << " - ";

    if (msg->type == DATA_INT) {
        std::cout << "INT - ";

        char sign = *data;
        data++;
        uint32_t val = ntohl(*(uint32_t *) data);

        std::cout << ((sign) ? "-" : "") << val << std::endl;
        return;
    }

    if (msg->type == DATA_SHORT_REAL) {
        std::cout << "SHORT_REAL - ";

        uint16_t val_short = ntohs(*(uint16_t*)data);
        float res = val_short / 100.0;
        std::cout << std::fixed << std::setprecision(2) << res << std::endl;
        return;
    }

    if (msg->type == DATA_FLOAT) {
        std::cout << "FLOAT - ";

        char sign = *data;
        data++;
        uint32_t val = ntohl(*(uint32_t*)data);
        data += sizeof(uint32_t);
        uint8_t exp = *data;
        std::cout << std::fixed << std::setprecision(exp);

        auto res = (double) val;
        while (exp) {
            res /= 10;
            exp--;
        }

        std::cout << ((sign) ? "-" : "") << res << std::endl;
        return;
    }

    if (msg->type == DATA_STRING) {
        std::cout << "STRING - ";

        std::cout << data << std::endl;
        return;
    }
}

void TCPClient::sendUnSubscribeMessage(std::string topic) {
    char buf[MAX_MSG_LEN];
    uint16_t buf_len = 0;

    auto data = unsub_msg();
    memcpy(data.topic_name, topic.c_str(), TOPIC_MAX_LEN);

    buf_len += sizeof(msg_hdr);
    buf_len += topic.size() + 1;

    msg_hdr msg{TYPE_UNSUB, htons(buf_len)};

    memcpy(buf, &msg, sizeof(msg_hdr));
    memcpy(buf + sizeof(msg_hdr), &data, sizeof(data));

    Socket::sendBuffer(server_socket->getFd(), buf, buf_len);

    std::cout << "Unsubscribed from topic." << std::endl;
}
