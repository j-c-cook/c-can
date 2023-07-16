#ifndef LINUX_CAN_LOGGER_H
#define LINUX_CAN_LOGGER_H

#include <stdint.h>
#include <can/io/blf/blf.h>

struct Logger_vtable {
    void (*create_logger)(void *);
    void (*on_message_received)(void *, struct Message *);
    void (*stop_logger)(void *);
};

struct Logger {
    char * file_name;
    FILE * file;
    uint16_t channel;

    void * writer;
    struct Logger_vtable methods;
};

struct Logger generic_get_logger(char * file_name);
void generic_on_message_received(struct Logger * logger, struct Message *can_msg);
void generic_stop_logger(struct Logger * logger);

struct RotatingLogger {
    // Size
    uint64_t max_bytes;
    // Time
    double last_rollover_time;
    uint64_t delta_t;

    uint32_t rollover_count;

    // Logger specific details
    struct BLFWriter * logger;
    int s;
    char * channel;

    // zlib compression information
    uint8_t compression_level;
};

struct RotatingLogger * create_rotating_logger(
        char * channel,
        char * file_name,
        uint64_t max_bytes,
        uint64_t delta_t,
        uint8_t compression_level);

void log_msg(struct RotatingLogger *);

void shutdown_rotating(struct RotatingLogger * r_logger);

#endif //LINUX_CAN_LOGGER_H
