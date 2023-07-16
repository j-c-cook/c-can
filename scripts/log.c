#include <can/bus.h>
#include <can/message.h>
#include <can/logger.h>


int main() {
    struct Logger logger = create_logger("file.blf", NULL);

    int s = create_socket();
    bind_socket(s, "can0");

    int count = 0;

    while (count < 100000) {
        struct Message * msg = capture_message(s);

        if (msg != NULL)
            on_message_received(&logger, msg);

        free_message(msg);

        count += 1;
    }

    stop_logger(&logger);

    return 0;
}
