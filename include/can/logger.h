#ifndef LINUX_CAN_LOGGER_H
#define LINUX_CAN_LOGGER_H

#include <stdint.h>
#include <can/io/blf.h>

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
