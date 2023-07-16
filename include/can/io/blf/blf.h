#ifndef LINUX_CAN_BLF_H
#define LINUX_CAN_BLF_H

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include "can/message.h"
#include <string.h>
#include <zlib.h>

#define APPLICATION_ID 5
#define APPLICATION_MAJOR 0
#define APPLICATION_MINOR 0
#define APPLICATION_BUILD 0
#define BIN_LOG_MAJOR 2
#define BIN_LOG_MINOR 6
#define BIN_LOG_BUILD 8
#define BIN_LOG_PATCH 1

#define FILE_HEADER_SIZE 144

#define TIME_ONE_NANS 0x00000002

#define MAX_CONTAINER_SIZE (128 * 1024)

#define CAN_MSG_EXT 0x80000000

// Object Types
#define BLF_CAN_MESSAGE   1
#define BLF_LOG_CONTAINER 10

# define ZLIB_DEFLATE 2

void timestamp_to_systemtime(double timestamp, uint16_t systemtime[]);

struct BLFWriter {
    char * file_name;
    FILE * file;
    uint16_t channel;
    uint16_t compression_level;
    uint8_t buffer[MAX_CONTAINER_SIZE];
    uint32_t buffer_size;
    double start_timestamp;
    double stop_timestamp;
    uint64_t uncompressed_size;
    uint32_t object_count;
};

void * blf_create_logger(char * file_name);
void blf_on_message_received(void *,struct Message * can_msg);
void blf_rollover(void * logger, uint64_t filesize, const char * new_filename);
void blf_stop_logger(void * logger);


struct BLFWriter * create_logger(char * file_name);
void on_message_received(struct BLFWriter *,struct Message * can_msg);
void rollover(struct BLFWriter * logger, uint64_t filesize, const char * new_filename);
void stop_logger(struct BLFWriter * logger);

#endif //LINUX_CAN_BLF_H
