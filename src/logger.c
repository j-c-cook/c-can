#include <can/logger.h>
#include <can/bus.h>
#include <time.h>

struct RotatingLogger * create_rotating_logger(
        char * channel,
        char * file_name,
        uint64_t max_bytes,
        uint64_t delta_t,
        uint8_t compression_level) {
    struct RotatingLogger * r_logger = malloc(sizeof (struct RotatingLogger));
    r_logger->max_bytes = max_bytes;
    r_logger->delta_t = delta_t;
    r_logger->last_rollover_time = 0.0; // set first rollover time when equal to 0.0

    r_logger->rollover_count = 0;

    r_logger->logger = create_logger(file_name);
    r_logger->s = create_socket();
    r_logger->channel = channel;

    r_logger->compression_level = compression_level;

    bind_socket(r_logger->s, r_logger->channel);

    return r_logger;
}

void _default_filename(char * f_name, size_t max_len, const double timestamp, const uint32_t rollover_count, const char * channel) {
    // TODO: Change to Greenwich time
    time_t rawtime = (time_t)timestamp;
    struct tm ptr_time;
    (void) gmtime_r(&rawtime, &ptr_time);

    const uint8_t f_time_len = 20;
    char f_time[f_time_len];
    strftime(f_time, f_time_len, "%Y-%m-%dT%H%M%S", &ptr_time);

    const uint8_t f_end_len = 10;
    char f_end[f_end_len];
    snprintf (f_end, f_end_len, "_%04d.blf", rollover_count);

    const uint8_t f_start_len = 8;
    char f_start[f_start_len];
    snprintf(f_start, f_start_len, "%s_", channel);

    snprintf(f_name, max_len, "%s%s%s", f_start, f_time, f_end);
}

void log_msg(struct RotatingLogger * r_logger) {
    struct Message * msg = capture_message(r_logger->s);

    if (msg == NULL) {
        free(msg);
        return;
    }

    if (r_logger->last_rollover_time == 0.0)
        r_logger->last_rollover_time = msg->timestamp;

    // Check size and time limit
    const uint64_t filesize = ftell(r_logger->logger->file);
    bool file_over_size = filesize > r_logger->max_bytes;
    bool file_passed_time = (msg->timestamp - r_logger->last_rollover_time) > (double)r_logger->delta_t;
    if (file_over_size || file_passed_time) {
        const uint8_t f_name_len = 80;
        char f_name[f_name_len];
        _default_filename(f_name, f_name_len, msg->timestamp, r_logger->rollover_count, r_logger->channel);

        rollover(r_logger->logger, filesize, f_name);
        r_logger->last_rollover_time = msg->timestamp;
        r_logger->rollover_count += 1;
    }

    on_message_received(r_logger->logger, msg);

    free(msg);
}

void shutdown_rotating(struct RotatingLogger * r_logger) {
    const uint8_t f_name_len = 80;
    char f_name[f_name_len];
    _default_filename(f_name, f_name_len, r_logger->logger->stop_timestamp, r_logger->rollover_count, r_logger->channel);
    stop_logger(r_logger->logger);
    free(r_logger);
}