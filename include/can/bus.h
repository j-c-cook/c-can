#ifndef C_CAN_BUS_H
#define C_CAN_BUS_H

#include <can/message.h>
#include <can/errors.h>

struct Bus_vtable {
    c_can_err_t (*open)(void * interface, const char * channel);
    // TODO: Remove timeout argument from function definition (should be passed in through args)
    struct Message (*on_message_received)(void * interface, double timeout);
    int (*send)(void * interface, struct Message * can_msg);
    int (*close)(void * interface);
};

struct Bus {
    const char * channel;
    u_int16_t channel_idx;
    const char * interface_name;
    void * interface;
    struct Bus_vtable methods;
    void * args;
};

c_can_err_t bus_configure(
        struct Bus * bus,
        char * interface_name,
        const char * channel,
        u_int16_t channel_idx,
        void * args);
struct Message bus_recv(struct Bus * bus, double timeout);
int bus_shutdown(struct Bus * bus);

#endif //C_CAN_BUS_H
