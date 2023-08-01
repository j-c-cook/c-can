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

    int s = create_socket();
    bind_socket(s, "can0");

    uint16_t t_stamp[8];

    struct Message * msg;
    while (!done) {
        msg = capture_message(s);

        if (msg != NULL) {
            timestamp_to_systemtime(msg->timestamp, t_stamp);
            for (int i=0; i<8; i++) {
                printf("%i ", t_stamp[i]);
            }
            print_message(msg);
        } // fi

        free_message(msg);
    }

    if (msg != NULL)
        free_message(msg);

    close_socket(s);

    return 0;
}
