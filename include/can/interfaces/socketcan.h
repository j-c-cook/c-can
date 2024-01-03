#ifndef C_CAN_SOCKETCAN_H
#define C_CAN_SOCKETCAN_H

#include <can/message.h>
#include <can/errors.h>
#include <can/bus.h>

struct SocketCanArgs {
    bool hwtimestamp;  // SO_TIMESTAMPING if true, else SO_TIMESTAMP
    struct timeval tv; // Socket read timeout
};

struct SocketCan {
    // Created during configuration
    int sock;
    // Passed in through args
    bool hwtimestamp;
    struct timeval tv;
};

c_can_err_t socketcan_configure(struct Bus * bus);
// TODO: Remove this from public facing API
void set_socket_timeout(struct SocketCan * socket_can, struct timeval tv);

#endif //C_CAN_SOCKETCAN_H
