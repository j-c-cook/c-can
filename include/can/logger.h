#ifndef LINUX_CAN_LOGGER_H
#define LINUX_CAN_LOGGER_H

#include <stdint.h>
#include <can/io/blf/blf.h>

struct Logger_vtable {
    void (*create_logger)(void *, void*);
    void (*on_message_received)(void *, struct Message *);
    void (*stop_logger)(void *);
};

struct Logger {
    char * file_name;
    FILE * file;

    void * writer;
    struct Logger_vtable methods;
    void * args;
};

struct Logger create_logger(char * file_name, void *args);
void on_message_received(struct Logger * logger, struct Message *can_msg);
void stop_logger(struct Logger * logger);

struct RotatingLogger_vtable {
    void (*rollover)(void *, const uint64_t filesize, const char * new_filename);
};

struct RotatingLogger {
    // Size (bytes)
    uint64_t max_bytes;
    // Time (seconds)
    double last_rollover_time;
    uint64_t delta_t;

    uint32_t rollover_count;

    struct Logger logger;
    struct RotatingLogger_vtable methods;
};

struct RotatingLogger create_rotating_logger(
        char * file_name,
        uint64_t max_bytes,
        uint64_t delta_t,
        void * args);

void log_msg(struct RotatingLogger * r_logger, struct Message * msg);

void shutdown_rotating(struct RotatingLogger * r_logger);

#endif //LINUX_CAN_LOGGER_H
