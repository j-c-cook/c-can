#ifndef LINUX_CAN_BLF_H
#define LINUX_CAN_BLF_H

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <can/message.h>
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

#define CAN_MAX_DLC 8

#define TIME_ONE_NANS 0x00000002

#define MAX_CONTAINER_SIZE (128 * 1024)

#define CAN_MSG_EXT 0x80000000

// Object Types
#define BLF_CAN_MESSAGE   1
#define BLF_LOG_CONTAINER 10

# define ZLIB_DEFLATE 2

void timestamp_to_systemtime(double timestamp, uint16_t systemtime[]);

// FILE_HEADER_STRUCT = struct.Struct("<4sLBBBBBBBBQQLL8H8H")
struct Header {
    uint8_t signature[4];
    uint32_t size;
    uint8_t info[8];
    uint64_t file_size;
    uint64_t uncompressed_size;
    uint32_t object_count;
    uint32_t count_of_objects_read;
    uint16_t start_timestamp[8];
    uint16_t stop_timestamp[8];
};

// CAN_MSG_STRUCT = struct.Struct("<HBBL8s")
struct CANMessage {
    uint16_t channel;
    uint8_t flags;
    uint8_t dlc;
    uint32_t arbitration_id;
    uint8_t data[CAN_MAX_DLC];
};

// OBJ_HEADER_BASE_STRUCT = struct.Struct("<4sHHLL")
struct Obj_Header_Base {
    char signature[4];
    uint16_t header_size;
    uint16_t header_version;
    uint32_t object_size;
    uint32_t object_type;
};

// OBJ_HEADER_V1_STRUCT = struct.Struct("<LHHQ")
struct Obj_Header_v1_Base {
    uint32_t flags;
    uint16_t client_index;
    uint16_t object_version;
    uint64_t timestamp;
};

// LOG_CONTAINER_STRUCT = struct.Struct("<H6xL4x")
#define LC_GAP0 6
#define LC_GAP1 4
struct Log_Container {
    uint16_t compression_method;
    uint8_t gap_0[LC_GAP0];
    uint32_t size_uncompressed;
    uint8_t gap_1[LC_GAP1];
};

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

struct BLFWriter * create_logger(char * file_name);
void _write_header(struct BLFWriter *, uint64_t filesize);
void on_message_received(struct BLFWriter *,struct Message * can_msg);
void _add_object(struct BLFWriter *, uint32_t obj_type, uint8_t data[], ssize_t data_size, double timestamp);
void _flush(struct BLFWriter *);
void rollover(struct BLFWriter * logger, uint64_t filesize, const char * new_filename);
void stop_logger(struct BLFWriter * logger);

#endif //LINUX_CAN_BLF_H
