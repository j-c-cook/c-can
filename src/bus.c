// c-can - a CAN hardware interface and file logging library written in C.
// Copyright (C) 2023 Jack Cook
// https://github.com/j-c-cook/c-can/blob/main/LICENSE

#include <can/bus.h>
#include <can/interfaces/socketcan.h>

#include <string.h>

c_can_err_t bus_configure(
        struct Bus * bus,
        char * interface_name,
        const char * channel,
        u_int16_t channel_idx,
        void * args) {
    bus->channel = channel;
    bus->interface_name = interface_name;
    bus->channel_idx = channel_idx;
    bus->args = args;

    // Setup bus
    c_can_err_t err;
    if (strcmp("socketcan", interface_name) == 0) {
        err = socketcan_configure(bus);
    } else {
        return INVALID_PARAMETERS;
    }

    // Handle return type from unique bus configuration
    if (err != SUCCESS) {
        return err;
    }

    bus->methods.open(bus->interface, bus->channel);

    return SUCCESS;
}

struct Message bus_recv(struct Bus * bus, double timeout) {
    struct Message can_msg = bus->methods.on_message_received(bus->interface, timeout);
    can_msg.channel = bus->channel_idx;
    return can_msg;
}

int bus_shutdown(struct Bus * bus) {
    return bus->methods.close(bus->interface);
}
