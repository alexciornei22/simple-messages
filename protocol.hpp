
#ifndef HOMEWORK2_PUBLIC_PROTOCOL_HPP
#define HOMEWORK2_PUBLIC_PROTOCOL_HPP

#include <cstdint>

#define TYPE_CONN 0
#define TYPE_SUB 1
#define TYPE_UNSUB 2
#define TYPE_TOPDAT 3
#define MAX_MSG_DATA_LEN 1600
#define ID_MAX_LEN 11

struct __attribute__((__packed__)) msg_hdr {
    uint8_t type;
    uint16_t msg_len;
};

#endif //HOMEWORK2_PUBLIC_PROTOCOL_HPP
