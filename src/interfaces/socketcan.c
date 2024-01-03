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
#include <linux/net_tstamp.h>

#include <stdio.h>
#include <errno.h>

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

void fill_message(struct Message * msg, struct canfd_frame * frame, struct timeval *tv) {
    const double us_to_s = 1e-6;
    msg->timestamp = (double)tv->tv_sec + (double)tv->tv_usec * us_to_s;
    msg->arbitration_id = frame->can_id;
    msg->dlc = frame->len;

    for (int i = 0; i < frame->len; i++)
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
    struct Message my_msg;

    struct timeval tv;

    struct msghdr msg;
    struct canfd_frame frame;
    struct sockaddr_can addr;
    struct iovec iov;
    char ctrlmsg[CMSG_SPACE(sizeof(struct timeval) + 3 * sizeof(struct timespec) + sizeof(__u32))];

    iov.iov_base = &frame;
    msg.msg_name = &addr;
    msg.msg_iov = &iov;
    msg.msg_iovlen = 1;
    msg.msg_control = &ctrlmsg;

    /* these settings may be modified by recvmsg() */
    iov.iov_len = sizeof(frame);
    msg.msg_namelen = sizeof(addr);
    msg.msg_controllen = sizeof(ctrlmsg);
    msg.msg_flags = 0;

    ssize_t nbytes = recvmsg(sock, &msg, 0);

    if (nbytes < 0) {
        perror("read");
        my_msg._recv_error = true;
        return my_msg;
    }
    my_msg._recv_error = false;

    struct cmsghdr *cmsg;
    for (cmsg = CMSG_FIRSTHDR(&msg);
         cmsg && (cmsg->cmsg_level == SOL_SOCKET);
         cmsg = CMSG_NXTHDR(&msg,cmsg)) {
        if (cmsg->cmsg_type == SO_TIMESTAMP) {
            memcpy(&tv, CMSG_DATA(cmsg), sizeof(tv));
        } else if (cmsg->cmsg_type == SO_TIMESTAMPING) {

            struct timespec *stamp = (struct timespec *) CMSG_DATA(cmsg);

            /*
             * stamp[0] is the software timestamp
             * stamp[1] is deprecated
             * stamp[2] is the raw hardware timestamp
             * See chapter 2.1.2 Receive timestamps in
             * linux/Documentation/networking/timestamping.txt
             */
            tv.tv_sec = stamp[2].tv_sec;
            tv.tv_usec = stamp[2].tv_nsec / 1000;
        }
    }

    fill_message(&my_msg, &frame, &tv) ;

    return my_msg;
}

int close_socket(int s) {
    int n;
    if ((n = close(s)) < 0) {
        perror("Close");
    }
    return n;
}

c_can_err_t socketcan_startup(void * interface, const char * channel) {
    struct SocketCan * socket_can = (struct SocketCan*)interface;

    // TODO: Handle error from create socket
    socket_can->sock = create_socket();

    // TODO: Handle error from setting socket option
    set_socket_timeout(socket_can, socket_can->tv);

    if (socket_can->hwtimestamp) {
        const int timestamping_flags = (SOF_TIMESTAMPING_SOFTWARE |
                                        SOF_TIMESTAMPING_RX_SOFTWARE |
                                        SOF_TIMESTAMPING_RAW_HARDWARE);

        if (setsockopt(socket_can->sock, SOL_SOCKET, SO_TIMESTAMPING,
                       &timestamping_flags, sizeof(timestamping_flags)) < 0) {
            perror("setsockopt SO_TIMESTAMPING is not supported by your Linux kernel");
            return NOT_SUPPORTED;
        }
    } else {
        const int timestamp_on = 1;

        if (setsockopt(socket_can->sock, SOL_SOCKET, SO_TIMESTAMP,
                       &timestamp_on, sizeof(timestamp_on)) < 0) {
            perror("setsockopt SO_TIMESTAMP");
            return FAILED_OPERATION;
        }
    }

    // TODO: Handle error from socket bind
    bind_socket(socket_can->sock, channel);

    return SUCCESS;
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

void set_socket_timeout(struct SocketCan * socket_can, struct timeval tv) {
    // TODO: Return error from this function
    if (setsockopt(
            socket_can->sock,
            SOL_SOCKET,
            SO_RCVTIMEO,
            (const char*)&tv,sizeof tv) < 0) {
        int errorno = errno;
        printf("setsockopt failed with error: %s (errno: %d)\n",
               strerror(errorno), errorno);
    }
}

c_can_err_t socketcan_configure(struct Bus * bus) {
    // Allocate memory for the interface (Note: Should be free'd in `close`)
    struct SocketCan * socket_can = malloc(sizeof (struct SocketCan));
    if (socket_can == NULL) {
        fprintf(stderr, "Failed to allocate memory\n");
        return OUT_OF_MEMORY;
    }
    bus->interface = socket_can;

    // Handle unique arguments
    if (bus->args == NULL) {
        socket_can->hwtimestamp = false; // Default to software timestamp

        socket_can->tv.tv_sec = 1;  /* 1 Sec Timeout */
        socket_can->tv.tv_usec = 0; // Not init'ing this can cause strange errors
    } else {
        struct SocketCanArgs *args = (struct SocketCanArgs*)bus->args;

        socket_can->hwtimestamp = args->hwtimestamp;

        socket_can->tv.tv_sec = args->tv.tv_sec;
        socket_can->tv.tv_usec = args->tv.tv_usec;
    }

    bus->methods.open = &socketcan_startup;
    bus->methods.on_message_received = &socketcan_recv;
    bus->methods.send = &socketcan_send;
    bus->methods.close = &socketcan_shutdown;

    return SUCCESS;
}
