#ifndef C_CAN_BUS_H
#define C_CAN_BUS_H

#include <can/message.h>

struct Bus_vtable {
    int (*open)(void * interface, const char * channel);
    struct Message (*on_message_received)(void * interface, double timeout);
    // TODO: type (*send)(args);
    int (*close)(void * interface);
};

struct Bus {
    const char * channel;
    u_int16_t channel_idx;
    const char * interface_name;
    void * interface;
    struct Bus_vtable methods;
    bool _configure_success;
};

struct Bus bus_configure(char * interface_name, const char * channel, u_int16_t channel_idx, void * args);
struct Message bus_recv(struct Bus * bus, double timeout);
int bus_shutdown(struct Bus * bus);

#endif //C_CAN_BUS_H
