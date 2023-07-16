#include <can/logger.h>
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

    struct RotatingLogger * r_logger = create_rotating_logger(
            "can0", "file.io", 1000000, 300, Z_BEST_COMPRESSION);

    while (!done) {
        log_msg(r_logger);
    }

    shutdown_rotating(r_logger);

    return 0;
}
