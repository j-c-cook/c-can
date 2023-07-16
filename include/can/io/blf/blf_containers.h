/**
 * Containers that are private to the BLF format.
 */

#ifndef C_CAN_BLF_CONTAINERS_H
#define C_CAN_BLF_CONTAINERS_H

#include <stdint.h>

#define CAN_MAX_DLC 8

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

#endif //C_CAN_BLF_CONTAINERS_H
