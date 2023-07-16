#include <can/bus.h>
#include <can/message.h>
#include <can/logger.h>


int main() {
    struct BLFWriterArgs args = {
            .compression_level = Z_BEST_COMPRESSION,
    };

    struct Logger logger = create_logger("file.blf", "can0", (void*)&args);

    int count = 0;

    while (count < 100000) {
        struct Message * msg = capture_message(logger.s);

        if (msg != NULL)
            on_message_received(&logger, msg);

        free_message(msg);

        count += 1;
    }

    stop_logger(&logger);

    return 0;
}
