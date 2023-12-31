// c-can - a CAN hardware interface and file logging library written in C.
// Copyright (C) 2023 Jack Cook
// https://github.com/j-c-cook/c-can/blob/main/LICENSE

#include <can/interfaces/socketcan.h>
#include <can/message.h>
#include <can/bus.h>

#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <net/if.h>
#include <sys/ioctl.h>
#include <sys/socket.h>

#include <linux/can.h>
#include <linux/can/raw.h>
#include <linux/sockios.h>

#include <stdio.h>

int create_socket() {
    int s;
    if ((s = socket(PF_CAN, SOCK_RAW, CAN_RAW)) < 0) {
        perror("Socket");
    }
    return s;
}

int bind_socket(int sock, const char * channel) {
    struct ifreq ifr;
    struct sockaddr_can addr;

    printf("Bind to %s\n", channel);

    strcpy(ifr.ifr_name, channel );
    ioctl(sock, SIOCGIFINDEX, &ifr);

    memset(&addr, 0, sizeof(addr));
    addr.can_family = AF_CAN;
    addr.can_ifindex = ifr.ifr_ifindex;

    int bound;
    if ((bound = bind(sock, (struct sockaddr *)&addr, sizeof(addr)) < 0)) {
        perror("Bind");
    }
    return bound;
}

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

void fill_frame(struct Message * msg, struct can_frame * frame) {
    frame->can_id = msg->arbitration_id;
    frame->can_dlc = msg->dlc;

    for (int i=0; i<frame->can_dlc; i++) {
        frame->data[i] = msg->data[i];
    }

    if (msg->is_extended_id)
        frame->can_id |= CAN_EFF_FLAG;

    if (msg->is_remote_frame)
        frame->can_id |= CAN_RTR_FLAG;

    if (msg->is_error_frame)
        frame->can_id |= CAN_ERR_FLAG;
}

struct Message capture_message(int sock) {
    // TODO: Use recv_msg so that the error information can be captured
    struct Message msg;

    struct can_frame frame;
    struct timeval tv;
    ssize_t nbytes = recv(sock, &frame, sizeof (struct can_frame), CAN_MTU);

    if (nbytes < 0) {
        msg._recv_error = true;
        return msg;
    }

    ioctl(sock, SIOCGSTAMP, &tv);
    fill_message(&msg, &frame, &tv);

    return msg;
}

int close_socket(int s) {
    int n;
    if ((n = close(s)) < 0) {
        perror("Close");
    }
    return n;
}

int socketcan_startup(void * interface, const char * channel) {
    struct SocketCan * socket_can = (struct SocketCan*)interface;

    socket_can->sock = create_socket();

    return bind_socket(socket_can->sock, channel);
}

int socketcan_send(void * interface, struct Message * can_msg) {
    struct SocketCan * socket_can = (struct SocketCan *)interface;

    struct can_frame frame;
    fill_frame(can_msg, &frame);

    ssize_t nbytes = send(socket_can->sock, &frame, sizeof (struct can_frame), CAN_MTU);

    return 0;
}

struct Message socketcan_recv(void * interface, double timeout) {
    struct SocketCan * socket_can = (struct SocketCan*)interface;

    return capture_message(socket_can->sock);
}

int socketcan_shutdown(void * interface) {
    struct SocketCan * socket_can = (struct SocketCan*)interface;

    close_socket(socket_can->sock);

    free(socket_can);

    return 0;
}

void socketcan_configure(void * _bus, void * args) {
    struct Bus * bus = (struct Bus*)_bus;
    // Note: Currently no args struct for SocketCAN

    struct SocketCan * socket_can = malloc(sizeof (struct SocketCan));
    bus->interface = socket_can;

    bus->methods.open = &socketcan_startup;
    bus->methods.on_message_received = &socketcan_recv;
    bus->methods.send = &socketcan_send;
    bus->methods.close = &socketcan_shutdown;

    bus->_configure_success = true;
}

void set_socket_timeout(struct SocketCan * socket_can, struct timeval tv) {
    setsockopt(socket_can->sock,SOL_SOCKET,SO_RCVTIMEO,(const char*)&tv,sizeof tv);
}
