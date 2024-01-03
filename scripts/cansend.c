#include <can/c_can.h>

int main() {

    struct Bus bus = bus_configure("socketcan", "can0", 1, NULL);

    struct Message can_msg;
    can_msg.arbitration_id = 0x18eefffe;
    can_msg.dlc = 8;
    can_msg.is_extended_id = 1;
    for (int i=0; i<8; i++) {
        can_msg.data[i] = i;
    }

    bus.methods.send(bus.interface, &can_msg);

    bus.methods.close(bus.interface);

    return 0;
}