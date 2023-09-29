#ifndef C_CAN_SOCKETCAN_H
#define C_CAN_SOCKETCAN_H

#include <can/message.h>

struct SocketCan {
    int sock;
};

void socketcan_configure(void * bus, void * args);
void set_socket_timeout(struct SocketCan * socket_can, struct timeval tv);

#endif //C_CAN_SOCKETCAN_H
