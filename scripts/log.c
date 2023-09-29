#include <can/c_can.h>


int main() {
    struct BLFWriterArgs args = {
            .compression_level = Z_DEFAULT_COMPRESSION,
    };

    struct Bus bus = bus_configure("socketcan", "can0", 1, NULL);
    struct Logger logger = create_logger("file.blf", (void*)&args);

    int count = 0;

    while (count < 100) {
        struct Message msg = bus_recv(&bus, 0.0);

        if (msg._recv_error == false)
            on_message_received(&logger, &msg);

        count += 1;
    }

    stop_logger(&logger);

    return 0;
}
