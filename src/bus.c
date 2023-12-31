// c-can - a CAN hardware interface and file logging library written in C.
// Copyright (C) 2023 Jack Cook
// https://github.com/j-c-cook/c-can/blob/main/LICENSE

#include <can/bus.h>
#include <can/interfaces/socketcan.h>

#include <string.h>

struct Bus bus_configure(char * interface_name, const char * channel, u_int16_t channel_idx, void * args) {
    struct Bus bus;

    bus.channel = channel;
    bus.interface_name = interface_name;
    bus.channel_idx = channel_idx;
    if (strcmp("socketcan", interface_name) == 0) {
        socketcan_configure((void*)&bus, args);
    } else {
        bus._configure_success = false;
        return bus;
    }

    bus.methods.open(bus.interface, bus.channel);

    return bus;
}

struct Message bus_recv(struct Bus * bus, double timeout) {
    struct Message can_msg = bus->methods.on_message_received(bus->interface, timeout);
    can_msg.channel = bus->channel_idx;
    return can_msg;
}

int bus_shutdown(struct Bus * bus) {
    return bus->methods.close(bus->interface);
}
