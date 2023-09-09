#include <can/c_can.h>
#include <signal.h>

volatile sig_atomic_t done = 0;

void term(int signum)
{
    done = 1;
}

int main() {
    struct sigaction action;
    memset(&action, 0, sizeof(action));
    action.sa_handler = term;
    sigaction(SIGTERM, &action, NULL);
    sigaction(SIGHUP, &action, NULL);

    struct Bus bus = bus_configure("socketcan", "can0", NULL);

    struct BLFWriterArgs args = {
            .compression_level = Z_DEFAULT_COMPRESSION,
    };

    struct RotatingLogger r_logger = create_rotating_logger(
            "file.blf", 250000, 300, (void*)&args);

    while (!done) {
        struct Message msg = bus_recv(&bus, 0.0);

        if (msg._recv_error == false)
            log_msg(&r_logger, &msg);
    }

    shutdown_rotating(&r_logger);

    return 0;
}
