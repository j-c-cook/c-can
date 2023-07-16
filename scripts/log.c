#include <can/bus.h>
#include <can/message.h>
#include <can/logger.h>


int main() {
    struct Logger logger = generic_get_logger("file.io");

    int s = create_socket();
    bind_socket(s, "can0");

    int count = 0;

    while (count < 100000) {
        struct Message * msg = capture_message(s);

        if (msg != NULL)
            generic_on_message_received(&logger, msg);

        free_message(msg);

        count += 1;
    }

    generic_stop_logger(&logger);

    return 0;
}
