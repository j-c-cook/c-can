// c-can - a CAN hardware interface and logging library written in C.
// Copyright (C) 2023 Diligent Code LLC
// https://github.com/diligentcode/c-can/blob/main/LICENSE

#include <can/logger.h>
#include <can/bus.h>
#include <time.h>

const char *get_filename_ext(const char *file_name) {
    // https://stackoverflow.com/a/5309508
    const char *dot = strrchr(file_name, '.');
    if(!dot || dot == file_name) return "";
    return dot + 1;
}

struct Logger create_logger(char * file_name, void *args) {
    struct Logger logger;

    logger.file_name = file_name;
    logger.args = args;

    // Set the virtual functions
    const char * b = get_filename_ext(file_name);
    if (strcmp(b, "blf") == 0) {
        logger.methods.create_logger = &blf_create_logger;
        logger.methods.on_message_received = &blf_on_message_received;
        logger.methods.stop_logger = &blf_stop_logger;
    } else {
        logger.methods.create_logger = NULL;
        logger.methods.on_message_received = NULL;
        logger.methods.stop_logger = NULL;
    }

    // Initialize the writer
    if (logger.methods.create_logger != NULL) {
        logger.methods.create_logger(&logger, logger.args);
    } else {
        logger.writer = NULL;
    }

    return logger;
}

void on_message_received(struct Logger * logger, struct Message *can_msg){
    logger->methods.on_message_received(logger, can_msg);
}

void stop_logger(struct Logger * logger){
    logger->methods.stop_logger(logger);
}

struct RotatingLogger create_rotating_logger(
        char * file_name,
        uint64_t max_bytes,
        uint64_t delta_t,
        void * args) {
    struct RotatingLogger r_logger;
    r_logger.max_bytes = max_bytes;
    r_logger.delta_t = delta_t;
    r_logger.last_rollover_time = 0.0; // set first rollover time when equal to 0.0

    r_logger.rollover_count = 0;

    r_logger.logger = create_logger(file_name, args);

    r_logger.methods.rollover = &blf_rollover;

    return r_logger;
}

void default_filename_(char * rollover_name, char * file_name, size_t max_len, const uint32_t rollover_count) {
    time_t timestamp = time(NULL); // seconds since January 1, 1970
    struct tm ptr_time;
    (void) gmtime_r(&timestamp, &ptr_time);

    const uint8_t f_time_len = 20;
    char f_time[f_time_len];
    strftime(f_time, f_time_len, "%Y-%m-%dT%H%M%S", &ptr_time);

    const uint8_t f_end_len = 10;
    char f_end[f_end_len];
    const char * file_ext = get_filename_ext(file_name);
    snprintf (f_end, f_end_len, "_%04d.%s", rollover_count, file_ext);

    // TODO: (1) automatically find file path stem and (2) allow full file paths to be used
    //       r_logger->logger.file_name
    const uint8_t f_start_len = 8;
    char f_start[f_start_len];
    snprintf(f_start, f_start_len, "%s_", "file");

    snprintf(rollover_name, max_len, "%s%s%s", f_start, f_time, f_end);
}

void log_msg(struct RotatingLogger * r_logger, struct Message * msg) {
    if (r_logger->last_rollover_time == 0.0)
        r_logger->last_rollover_time = msg->timestamp;

    // Check size and time limit
    const uint64_t filesize = ftell(r_logger->logger.file);
    bool file_over_size = filesize > r_logger->max_bytes;
    bool file_passed_time = (msg->timestamp - r_logger->last_rollover_time) > (double)r_logger->delta_t;
    if (file_over_size || file_passed_time) {
        const uint8_t f_name_len = 80;
        char rollover_name[f_name_len];
        default_filename_(
                rollover_name,
                r_logger->logger.file_name,
                f_name_len,
                r_logger->rollover_count);

        r_logger->methods.rollover(&r_logger->logger, filesize, rollover_name);
        r_logger->last_rollover_time = msg->timestamp;
        r_logger->rollover_count += 1;
    }

    on_message_received(&r_logger->logger, msg);
}

void shutdown_rotating(struct RotatingLogger * r_logger) {
    const uint8_t f_name_len = 80;
    char rollover_name[f_name_len];
    default_filename_(rollover_name,
                      r_logger->logger.file_name,
                      f_name_len,
                      r_logger->rollover_count);
    r_logger->logger.methods.stop_logger(&r_logger->logger);
}