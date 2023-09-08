#ifndef C_CAN_SOCKETCAN_H
#define C_CAN_SOCKETCAN_H

#include <can/message.h>

struct SocketCan {
    int sock;
};

void socketcan_configure(void * bus, void * args);

#endif //C_CAN_SOCKETCAN_H
