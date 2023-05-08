
#ifndef HOMEWORK2_PUBLIC_PROTOCOL_HPP
#define HOMEWORK2_PUBLIC_PROTOCOL_HPP

#include <cstdint>

/*
 * Types for the TCP messages
 */
#define TYPE_CONN 0
#define TYPE_SUB 1
#define TYPE_UNSUB 2
#define TYPE_TOPDAT 3

/*
 * Types for UDP message topic data
 */
#define DATA_INT 0
#define DATA_SHORT_REAL 1
#define DATA_FLOAT 2
#define DATA_STRING 3

#define MAX_MSG_LEN 1600
#define TOPIC_MAX_LEN 50
#define DATA_MAX_LEN 1500
#define ID_MAX_LEN 11

struct __attribute__((__packed__)) msg_hdr {
    uint8_t type:4, sf:4;
    uint16_t msg_len;
};

struct udp_msg {
    in_addr_t client_addr;
    in_port_t client_port;
    char topic[50];
    uint8_t type;
    char data[1500];
};

#endif //HOMEWORK2_PUBLIC_PROTOCOL_HPP
