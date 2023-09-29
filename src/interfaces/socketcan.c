// c-can - a CAN hardware interface and file logging library written in C.
// Copyright (C) 2023 Diligent Code LLC
// https://github.com/diligentcode/c-can/blob/main/LICENSE

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
    bus->methods.close = &socketcan_shutdown;

    bus->_configure_success = true;
}

void set_socket_timeout(struct SocketCan * socket_can, struct timeval tv) {
    setsockopt(socket_can->sock,SOL_SOCKET,SO_RCVTIMEO,(const char*)&tv,sizeof tv);
}
