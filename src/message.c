// c-can - a CAN hardware interface and file logging library written in C.
// Copyright (C) 2023 Jack Cook
// https://github.com/j-c-cook/c-can/blob/main/LICENSE

#include <can/message.h>
#include <stdio.h>

void print_message(struct Message * msg) {
    printf("%f 0x%03X [%d] ", msg->timestamp, msg->arbitration_id, msg->dlc);

    for (int i = 0; i < msg->dlc; i++)
        printf("%02X ", msg->data[i]);

    printf("\n");
}
