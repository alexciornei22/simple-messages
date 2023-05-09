
#ifndef HOMEWORK2_PUBLIC_PROTOCOL_HPP
#define HOMEWORK2_PUBLIC_PROTOCOL_HPP

#include <cstdint>

/*
 * Types for the TCP messages
 */
#define TYPE_CONN 0 // connection ID message
#define TYPE_SUB 1 // subscribe message
#define TYPE_UNSUB 2 // unsubscribe message
#define TYPE_TOPDAT 3 // topic data message

/*
 * Types for UDP message topic data
 */
#define DATA_INT 0
#define DATA_SHORT_REAL 1
#define DATA_FLOAT 2
#define DATA_STRING 3

#define MAX_MSG_LEN 1600
#define TOPIC_MAX_LEN 50
#define DATA_MAX_LEN 1501
#define ID_MAX_LEN 11

struct __attribute__((__packed__)) msg_hdr {
    uint8_t type;
    uint16_t msg_len;
};

struct sub_msg {
    uint8_t sf;
    char topic_name[TOPIC_MAX_LEN];
};

struct udp_msg {
    in_addr_t client_addr{};
    in_port_t client_port{};
    char topic[TOPIC_MAX_LEN]{};
    uint8_t type{};
    uint16_t size{};
    char data[DATA_MAX_LEN]{};
};

#endif //HOMEWORK2_PUBLIC_PROTOCOL_HPP
