#include <can/c_can.h>
#include <signal.h>
#include <can/interfaces/socketcan.h>


volatile sig_atomic_t done = 0;

void term(int signum)
{
    done = 1;
}

void monitor(struct Bus * can_bus, struct RotatingLogger * r_logger) {
    struct Message msg = bus_recv(can_bus, 0.0);

    if (msg._recv_error == false) {
        log_msg(r_logger, &msg);
        if (can_bus->channel_idx == 2) {
            // TODO: Check to see if there has been an address request
        } // fi
    } // fi
}

int main() {
    struct sigaction action;
    memset(&action, 0, sizeof(action));
    action.sa_handler = term;
    sigaction(SIGTERM, &action, NULL);
    sigaction(SIGHUP, &action, NULL);

    const uint8_t num_buses = 3;

    struct Bus buses[num_buses];
    buses[0] = bus_configure("socketcan", "can0", 1, NULL);
    buses[1] = bus_configure("socketcan", "can1", 2, NULL);
    buses[2] = bus_configure("socketcan", "can2", 3, NULL);

    // Set timeout for CAN socket reading
    struct timeval tv;
    tv.tv_sec = 0;
    tv.tv_usec = 1;
    for (int i=0; i<num_buses; i++) {
        set_socket_timeout((struct SocketCan*)buses[i].interface, tv);
    }

    struct BLFWriterArgs args = {
            .compression_level = Z_DEFAULT_COMPRESSION,
    };

    struct RotatingLogger r_logger = create_rotating_logger(
            "file.blf", 250000, 300, (void*)&args);

    // TODO: Send address claim from CAN1

    while (!done) {
        for (int i=0; i<num_buses; i++) {
            monitor(&buses[i], &r_logger);
        }
    }

    shutdown_rotating(&r_logger);

    return 0;
}
