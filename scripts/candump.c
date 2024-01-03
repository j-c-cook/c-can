#include <can/c_can.h>
#include <signal.h>

volatile sig_atomic_t done = 0;


void term(int signum)
{
    done = 1;
}


int main() {
    // Allow operator to cleanly kill program by hitting "CTRL+C"
    struct sigaction action;
    memset(&action, 0, sizeof(action));
    action.sa_handler = term;
    sigaction(SIGTERM, &action, NULL);
    sigaction(SIGHUP, &action, NULL);

    // Configure a bus interface
    struct Bus bus;
    c_can_err_t err = bus_configure(
            &bus,
            "socketcan",
            "can0",
            1,
            NULL);
    if (err != SUCCESS) {
        fprintf(stderr, "Bus configuration error\n");
        return 1;
    }

    // Continuously print CAN messages to console
    uint16_t t_stamp[8];

    struct Message msg;
    while (!done) {
        msg = bus_recv(&bus, 0.0);

        if (msg._recv_error != true) {
            timestamp_to_systemtime(msg.timestamp, t_stamp);
            for (int i=0; i<8; i++) {
                printf("%i ", t_stamp[i]);
            }
            print_message(&msg);
        } // fi
    }

    bus_shutdown(&bus);

    return 0;
}
