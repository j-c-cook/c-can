// c-can - a CAN hardware interface and file logging library written in C.
// Copyright (C) 2023 Diligent Code LLC
// https://github.com/diligentcode/c-can/blob/main/LICENSE

#include <can/message.h>
#include <stdio.h>

void fill_message(struct Message * msg, struct can_frame * frame, struct timeval *tv) {
    const double us_to_s = 1e-6;
    msg->timestamp = (double)tv->tv_sec + (double)tv->tv_usec * us_to_s;
    msg->arbitration_id = frame->can_id;
    msg->dlc = frame->can_dlc;

    for (int i = 0; i < frame->can_dlc; i++)
        msg->data[i] = frame->data[i];

    msg->is_extended_id = frame->can_id & CAN_EFF_FLAG;
    msg->is_remote_frame = frame->can_id & CAN_RTR_FLAG;
    msg->is_error_frame = frame->can_id & CAN_ERR_FLAG;

    if (msg->is_extended_id) {
        msg->arbitration_id = frame->can_id & 0x1FFFFFFF;
    } else {
        msg->arbitration_id = frame->can_id & 0x000007FF;
    }

    msg->_recv_error = false;

}

void print_message(struct Message * msg) {
    printf("%f 0x%03X [%d] ", msg->timestamp, msg->arbitration_id, msg->dlc);

    for (int i = 0; i < msg->dlc; i++)
        printf("%02X ", msg->data[i]);

    printf("\n");
}
