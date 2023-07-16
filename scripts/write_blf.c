#include "can/io/blf.h"
#include <can/message.h>

int main() {
    struct BLFWriter * logger = create_logger();

    struct Message msg;
    msg.timestamp = (double)1685225282.8679;
    msg.arbitration_id = 0x18ff4fd0;
    msg.dlc = 8;
    msg.data[0] = 0x1;
    msg.data[1] = 0x3;
    msg.data[2] = 0x82;
    msg.data[3] = 0x1;
    msg.data[4] = 0x45;
    msg.data[5] = 0x95;
    msg.data[6] = 0x25;
    msg.data[7] = 0xf;
    msg.is_extended_id = true;

    on_message_received(logger, &msg);

    msg.timestamp += 1;

    on_message_received(logger, &msg);

    stop_logger(logger);

    return 0;
}