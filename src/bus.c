#include <can/bus.h>
#include <can/message.h>
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
} // create_socket()

struct Message * capture_message(int sock) {
    // TODO: Use recv_msg so that the error information can be captured
    struct Message * msg = malloc(sizeof (struct Message));

    struct can_frame frame;
    struct timeval tv;
    ssize_t nbytes = recv(sock, &frame, sizeof (struct can_frame), CAN_MTU);
    ioctl(sock, SIOCGSTAMP, &tv);

    if (nbytes < 0) {
        perror("Read");
        return NULL;
    }

    fill_message(msg, &frame, &tv);

    return msg;
}

void free_message(struct Message * msg) {
    free(msg);
}

int close_socket(int s) {
    int n;
    if ((n = close(s)) < 0) {
        perror("Close");
    }
    return n;
} // close_socket()
