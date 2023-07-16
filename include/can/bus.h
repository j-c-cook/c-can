#ifndef LINUX_CAN_BUS_H
#define LINUX_CAN_BUS_H

#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <net/if.h>
#include <sys/ioctl.h>
#include <sys/socket.h>

#include <linux/can.h>
#include <linux/can/raw.h>
#include <linux/sockios.h>

int create_socket();
int bind_socket(int sock, const char * channel);
struct Message * capture_message(int sock);
void free_message(struct Message *);
int close_socket(int s);

#endif //LINUX_CAN_BUS_H
