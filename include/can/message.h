#ifndef LINUX_CAN_MESSAGE_H
#define LINUX_CAN_MESSAGE_H

#include <stdbool.h>
#include <linux/can.h>
#include <net/if.h>

struct Message {
    double timestamp;
    u_int32_t arbitration_id;
    u_int8_t dlc;
    u_int8_t data[CAN_MAX_DLEN];
    bool is_extended_id;
    bool is_remote_frame;
    bool is_error_frame;
};

void fill_message(struct Message * msg, struct can_frame * frame, struct timeval *tv);
void print_message(struct Message * msg);

#endif //LINUX_CAN_MESSAGE_H
