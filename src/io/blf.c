/**
 * Seeks to implement BLF (Binary Logging Format) read/write functionality. The BLF file format is
 * proprietary and was created by Vector Informatik GmbH (Germany).
 *
 * Vector does not provide a specification for the BLF format. The source code contained in this
 * module is an adaptation of the implementation at hardbyte's python-can branch, which was
 * developed by the python-can contributors
 * (https://github.com/hardbyte/python-can/blob/develop/can/io/blf.py) and is licenced under GPLv3.
 *
 */

#include <can/io/blf/blf.h>
#include <can/io/blf/blf_containers.h>
#include <time.h>
#include <math.h>

void timestamp_to_systemtime(double timestamp, uint16_t systemtime[]) {
    if (timestamp == 0.0 || timestamp < 631152000) {
        // Probably not a unix timestamp
        systemtime[0] = 0;
        systemtime[1] = 0;
        systemtime[2] = 0;
        systemtime[3] = 0;
        systemtime[4] = 0;
        systemtime[5] = 0;
        systemtime[6] = 0;
        systemtime[7] = 0;
    } else {
        const double seconds_in_year = 31536000;

        time_t time_raw_format = (time_t)timestamp;
        struct tm ptr_time;
        (void) gmtime_r(&time_raw_format, &ptr_time);

        systemtime[0] = (uint16_t)1970 + (uint16_t)floor(timestamp / seconds_in_year);
        systemtime[1] = ptr_time.tm_mon + 1;
        systemtime[2] = ptr_time.tm_wday;
        systemtime[3] = ptr_time.tm_mday;
        systemtime[4] = ptr_time.tm_hour;
        systemtime[5] = ptr_time.tm_min;
        systemtime[6] = ptr_time.tm_sec;
        systemtime[7] = (uint16_t)ceil((timestamp - floor(timestamp)) * 1e3);
    } // fi
}

void _write_header(struct BLFWriter * logger, uint64_t filesize) {
    struct Header header;

    header.signature[0] = 'L';
    header.signature[1] = 'O';
    header.signature[2] = 'G';
    header.signature[3] = 'G';

    header.size = FILE_HEADER_SIZE;

    header.info[0] = APPLICATION_ID;
    header.info[1] = APPLICATION_MAJOR;
    header.info[2] = APPLICATION_MINOR;
    header.info[3] = APPLICATION_BUILD;
    header.info[4] = BIN_LOG_MAJOR;
    header.info[5] = BIN_LOG_MINOR;
    header.info[6] = BIN_LOG_BUILD;
    header.info[7] = BIN_LOG_PATCH;

    header.file_size = filesize;
    header.uncompressed_size = logger->uncompressed_size;

    header.object_count = logger->object_count;
    header.count_of_objects_read = 0;

    timestamp_to_systemtime(logger->start_timestamp, header.start_timestamp);
    timestamp_to_systemtime(logger->stop_timestamp, header.stop_timestamp);

    fwrite(&header, sizeof(struct Header), 1, logger->file);

    uint16_t pad_size = (FILE_HEADER_SIZE - sizeof (header)) / 8;
    uint8_t padding[8] = {0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0};
    for (int i=0; i<pad_size; i++)
        fwrite(&padding, sizeof (uint8_t), 8, logger->file);
}

void _flush(struct BLFWriter * logger) {
    ssize_t obj_header_size = sizeof (struct Obj_Header_Base);
    ssize_t log_container_size = sizeof (struct Log_Container);

    uint8_t data[MAX_CONTAINER_SIZE];
    uint16_t compression_method;
    if (logger->compression_level == Z_NO_COMPRESSION) {
        memcpy(data, logger->buffer, logger->buffer_size);
        compression_method = Z_NO_COMPRESSION;
    } else {
        compress2(data,
                  (uLongf *) &logger->buffer_size,
                  (const Bytef *) logger->buffer,
                  logger->buffer_size,
                  logger->compression_level);
        compression_method = ZLIB_DEFLATE;
    }
    ssize_t data_size = logger->buffer_size;
    ssize_t obj_size = obj_header_size + log_container_size + data_size;

    struct Obj_Header_Base base_header;
    base_header.signature[0] = 'L';
    base_header.signature[1] = 'O';
    base_header.signature[2] = 'B';
    base_header.signature[3] = 'J';
    base_header.header_size = obj_header_size;
    base_header.header_version = 1;
    base_header.object_size = obj_size;
    base_header.object_type = BLF_LOG_CONTAINER;
    fwrite(&base_header, obj_header_size, 1, logger->file);

    struct Log_Container container_header;
    container_header.compression_method = compression_method;
    for (int i=0; i<LC_GAP0; i++)
        container_header.gap_0[i] = 0x0;
    container_header.size_uncompressed = data_size;
    for (int i=0; i<LC_GAP1; i++)
        container_header.gap_1[i] = 0x0;
    fwrite(&container_header, log_container_size, 1, logger->file);

    fwrite(data, sizeof (uint8_t), logger->buffer_size, logger->file);

    // Write padding bytes
    uint8_t gap = 0x0;
    for (int i=0; i<obj_size%4; i++)
        fwrite(&gap, sizeof (uint8_t), 1, logger->file);

    logger->uncompressed_size += obj_size;
    logger->buffer_size = 0;
}

void _add_object(struct BLFWriter * logger, uint32_t obj_type, uint8_t data[], ssize_t data_size, double timestamp) {
    ssize_t obj_header_size = sizeof (struct Obj_Header_Base);
    ssize_t obj_header_v1_size = sizeof (struct Obj_Header_v1_Base);
    ssize_t obj_size = obj_header_size + obj_header_v1_size + data_size;

    if (obj_size + logger->buffer_size > MAX_CONTAINER_SIZE) {
        _flush(logger);
    }

    if (logger->start_timestamp == 0.0)
        logger->start_timestamp = timestamp;
    logger->stop_timestamp = timestamp;
    timestamp = (timestamp - logger->start_timestamp) * 1e9;

    struct Obj_Header_Base base_header;
    base_header.signature[0] = 'L';
    base_header.signature[1] = 'O';
    base_header.signature[2] = 'B';
    base_header.signature[3] = 'J';
    base_header.header_size = obj_header_size + obj_header_v1_size;
    base_header.header_version = 1;
    base_header.object_size = obj_size;
    base_header.object_type = obj_type;

    memcpy(&logger->buffer[logger->buffer_size], &base_header, sizeof (base_header));
    logger->buffer_size += sizeof(base_header);

    struct Obj_Header_v1_Base obj_header;
    obj_header.flags = TIME_ONE_NANS;
    obj_header.client_index = 0;
    obj_header.object_version = 0;
    if (timestamp > 0)
        obj_header.timestamp = (uint64_t)timestamp; // max(timestamp, 0)
    else
        obj_header.timestamp = (uint64_t)0;

    memcpy(&logger->buffer[logger->buffer_size], &obj_header, sizeof (obj_header));
    logger->buffer_size += sizeof (obj_header);

    memcpy(&logger->buffer[logger->buffer_size], data, data_size);
    logger->buffer_size += data_size;

    logger->object_count += 1;
}

void * blf_create_logger(char * file_name) {
    struct BLFWriter * logger = malloc(sizeof (struct BLFWriter));

    logger->file_name = file_name;
    logger->file = fopen(logger->file_name, "wb");
    logger->channel = 1;
    logger->compression_level = 0;
    logger->buffer_size = 0;
    logger->start_timestamp = 0.0;
    logger->stop_timestamp = 0.0;

    logger->uncompressed_size = FILE_HEADER_SIZE;
    logger->object_count = 0;

    _write_header(logger, FILE_HEADER_SIZE);

    return logger;
}

void blf_on_message_received(void * logger, struct Message * can_msg) {
    struct CANMessage blf_message;
    blf_message.channel = 1;
    blf_message.arbitration_id = can_msg->arbitration_id;
    if (can_msg->is_extended_id)
        blf_message.arbitration_id |= CAN_MSG_EXT;
    blf_message.flags = 0;
    blf_message.dlc = can_msg->dlc;
    for (int i=0; i<CAN_MAX_DLEN; i++)
        blf_message.data[i] = can_msg->data[i];

    uint8_t can_msg_buffer[sizeof (struct CANMessage)];
    memcpy(can_msg_buffer, &blf_message, sizeof (struct CANMessage));

    _add_object(logger, BLF_CAN_MESSAGE, can_msg_buffer, sizeof (struct CANMessage), can_msg->timestamp);
}

void blf_rollover(void * logger, const uint64_t filesize, const char * new_filename) {
    _flush(logger);

    struct BLFWriter *blf_logger = (struct BLFWriter*)logger;

    fseek(blf_logger->file, 0, SEEK_SET);
    _write_header(logger, filesize);

    fclose(blf_logger->file);

    const char * src = blf_logger->file_name;
    const char * dest = new_filename;
    rename(src, dest);

    free(logger);
    logger = create_logger((char *)src);
}

void blf_stop_logger(void * logger) {
    _flush(logger);

    struct BLFWriter *blf_logger = (struct BLFWriter*)logger;

    uint64_t filesize = ftell(blf_logger->file);
    fseek(blf_logger->file, 0, SEEK_SET);
    _write_header(logger, filesize);

    fclose(blf_logger->file);
    free(logger);
}

struct BLFWriter * create_logger(char * file_name) {
    struct BLFWriter * logger = malloc(sizeof (struct BLFWriter));

    logger->file_name = file_name;
    logger->file = fopen(logger->file_name, "wb");
    logger->channel = 1;
    logger->compression_level = 0;
    logger->buffer_size = 0;
    logger->start_timestamp = 0.0;
    logger->stop_timestamp = 0.0;

    logger->uncompressed_size = FILE_HEADER_SIZE;
    logger->object_count = 0;

    _write_header(logger, FILE_HEADER_SIZE);

    return logger;
}

void on_message_received(struct BLFWriter * logger, struct Message * can_msg) {
    struct CANMessage blf_message;
    blf_message.channel = 1;
    blf_message.arbitration_id = can_msg->arbitration_id;
    if (can_msg->is_extended_id)
        blf_message.arbitration_id |= CAN_MSG_EXT;
    blf_message.flags = 0;
    blf_message.dlc = can_msg->dlc;
    for (int i=0; i<CAN_MAX_DLEN; i++)
        blf_message.data[i] = can_msg->data[i];

    uint8_t can_msg_buffer[sizeof (struct CANMessage)];
    memcpy(can_msg_buffer, &blf_message, sizeof (struct CANMessage));

    _add_object(logger, BLF_CAN_MESSAGE, can_msg_buffer, sizeof (struct CANMessage), can_msg->timestamp);
}

void rollover(struct BLFWriter * logger, const uint64_t filesize, const char * new_filename) {
    _flush(logger);

    fseek(logger->file, 0, SEEK_SET);
    _write_header(logger, filesize);

    fclose(logger->file);

    const char * src = logger->file_name;
    const char * dest = new_filename;
    rename(src, dest);

    free(logger);
    logger = create_logger((char *)src);
}

void stop_logger(struct BLFWriter * logger) {
    _flush(logger);

    uint64_t filesize = ftell(logger->file);
    fseek(logger->file, 0, SEEK_SET);
    _write_header(logger, filesize);

    fclose(logger->file);
    free(logger);
}
