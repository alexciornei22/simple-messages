#include <iostream>
#include <cstring>
#include <poll.h>
#include <unistd.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <sys/socket.h>
#include <vector>
#include "TCPServer.hpp"

#define DIE(assertion, call_description)				\
	do {								\
		if (assertion) {					\
			fprintf(stderr, "(%s, %d): ",			\
					__FILE__, __LINE__);		\
			perror(call_description);			\
			exit(errno);					\
		}							\
	} while (0)

using namespace std;

pollfd make_pollfd(int fd, short events, short revents)
{
    pollfd pfd{};
    pfd.fd = fd;
    pfd.events = events;
    pfd.revents = revents;

    return pfd;
}

int main(int argc, char **argv)
{
    setvbuf(stdout, nullptr, _IONBF, BUFSIZ);

//    vector<pollfd> pollfds;
//
//    pollfds.push_back(make_pollfd(STDIN_FILENO, POLLIN, 0));
//
//    string command;
    uint16_t port;
    int rc = sscanf(argv[1], "%hu", &port);
    DIE(rc < 0, "invalid port");
//
//    int listenfd = socket(AF_INET, SOCK_STREAM, -1);
//    DIE(listenfd < 0, "server_socket");
//
//    struct sockaddr_in serv_addr;
//    socklen_t socket_len = sizeof(sockaddr_in);
//
//    int enable = 1;
//    // Deactivate Nagle's algorithm
//    rc = setsockopt(listenfd, SOL_TCP, TCP_NODELAY, &enable, sizeof(int));
//    DIE(rc < 0, "setsockopt TCP_NODELAY");
//    rc = setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int));
//    DIE(rc < 0, "setsockopt SO_REUSEADDR");
//
//    memset(&serv_addr, 0, socket_len);
//    serv_addr.sin_family = AF_INET;
//    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
//    serv_addr.sin_port = htons(port);
//
//    rc = bind(listenfd, (const struct sockaddr *)&serv_addr, sizeof(serv_addr));
//    DIE(rc < 0, "bind");
//
//    rc = listen(listenfd, 100);
//    DIE(rc < 0, "listen");
//
//    pollfds.push_back(make_pollfd(listenfd, POLLIN, 0));
//
//    while (true) {
//        rc = poll(pollfds.data(), pollfds.size(), -1);
//        DIE(rc < 0, "poll");
//
//        for (pollfd pfd : pollfds) {
//            if (pfd.revents & POLLIN) {
//                if (pfd.fd == STDIN_FILENO) {
//                    getline(cin, command);
//                    if (command == "exit") {
//                        exit(EXIT_SUCCESS);
//                    }
//                }
//            }
//        }
//    }
    TCPServer server = TCPServer(port);
    server.run();
    return 0;
}