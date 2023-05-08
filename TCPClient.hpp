#ifndef PCOM_HW2_TCPCLIENT_HPP
#define PCOM_HW2_TCPCLIENT_HPP

#include <vector>
#include <sys/poll.h>
#include "Socket.hpp"

class TCPClient {
public:
    TCPClient(std::string ip, uint16_t port);
    ~TCPClient();
    void run(std::string id);

private:
    bool active = false;
    Socket *server_socket;
    sockaddr_in server_addr = sockaddr_in();
    std::vector<pollfd> poll_fds;

    void getConsoleCommands();
};


#endif //PCOM_HW2_TCPCLIENT_HPP
