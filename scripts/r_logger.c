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

    struct BLFWriterArgs args = {
            .compression_level = Z_BEST_COMPRESSION,
    };

    struct RotatingLogger * r_logger = create_rotating_logger(
            "can0", "file.blf", 1000000, 300, (void*)&args);

    while (!done) {
        log_msg(r_logger);
    }

    shutdown_rotating(r_logger);

    return 0;
}
