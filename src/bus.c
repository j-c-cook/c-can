// c-can - a CAN hardware interface and logging library written in C.
// Copyright (C) 2023 Diligent Code LLC
// https://github.com/diligentcode/c-can/blob/main/LICENSE

#include <can/bus.h>
#include <can/interfaces/socketcan.h>

#include <string.h>

struct Bus bus_configure(char * interface_name, const char * channel, void * args) {
    struct Bus bus;

    bus.channel = channel;
    bus.interface_name = interface_name;
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
    return bus->methods.on_message_received(bus->interface, timeout);
}

int bus_shutdown(struct Bus * bus) {
    return bus->methods.close(bus->interface);
}
