#include <can/c_can.h>
#include <signal.h>
#include <can/interfaces/socketcan.h>
#include <stdlib.h>


volatile sig_atomic_t done = 0;

void term(int signum)
{
    done = 1;
}

u_int32_t get_pgn(uint32_t frame_id) {
    // Note: pgn is a 17 bit value
    // The data page (DP) is contained in bit 0 of the msb
    u_int8_t data_page = (((frame_id >> 24) & 0x1f) >> 17) & 0x1;

    // The PDU format (PF) is located one byte to the right of msb
    u_int8_t pdu_format = (frame_id >> 16) & 0xFF;
    // The PDU specific (PS) is located one byte to the right of PF
    u_int8_t pdu_specific = (frame_id >> 8) & 0xFF;

    u_int32_t pgn = data_page << 16 | pdu_format << 8;

    if (pdu_format<=239) {} // PDU1 has PF from 0-239
    else if (pdu_format >= 240) pgn |= pdu_specific; // PDU2 has PF from 240-255

    return pgn;
}

void send_address_claim(struct Bus * can_bus, struct RotatingLogger * r_logger) {
    struct Message can_msg;
    can_msg.arbitration_id = 0x18eefff9;
    can_msg.dlc = 8;
    can_msg.is_extended_id = 1;
    can_msg.is_remote_frame = false;
    can_msg.is_error_frame = false;
    uint64_t data = 0x0000c00c00810000;
    for (int i=8; i>0; i--) {
        uint8_t shift_right = 64 - (8 * i);
        can_msg.data[i-1] = (data >> shift_right) & 0xff;
    }

    can_bus->methods.send(can_bus->interface, &can_msg);
    log_msg(r_logger, &can_msg);
}

void monitor(struct Bus * can_bus, struct RotatingLogger * r_logger) {
    struct Message msg = bus_recv(can_bus, 0.0);

    if (msg._recv_error == false) {
        log_msg(r_logger, &msg);
        if (can_bus->channel_idx == 2) {
            if (get_pgn(msg.arbitration_id) == 0xea00) {
                if (((msg.data[1] << 8) | msg.data[0]) == 0xee00)
                    send_address_claim(can_bus, r_logger);
            }
        } // fi
    } // fi
}

int main(int argc, char *argv[]) {
    // Read command line args
    const char * channel = argv[1];
    const char * file_name = argv[2];
    int channel_idx = atoi(argv[3]);

    struct sigaction action;
    memset(&action, 0, sizeof(action));
    action.sa_handler = term;
    sigaction(SIGTERM, &action, NULL);
    sigaction(SIGHUP, &action, NULL);

    struct Bus bus = bus_configure("socketcan", channel, channel_idx, NULL);

    // Set timeout for CAN socket reading
    struct timeval tv;
    tv.tv_sec = 0;
    tv.tv_usec = 1;
    set_socket_timeout((struct SocketCan*)bus.interface, tv);

    struct BLFWriterArgs args = {
            .compression_level = Z_DEFAULT_COMPRESSION,
    };

    struct RotatingLogger r_logger = create_rotating_logger(
            file_name, 1000000, 300, (void*)&args);

    send_address_claim(&bus, &r_logger);

    while (!done) {
        monitor(&bus, &r_logger);
    }

    shutdown_rotating(&r_logger);

    bus.methods.close(bus.interface);

    return 0;
}
