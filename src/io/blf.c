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
#include <time.h>
#include <math.h>

#include <can/io/blf/blf_containers.h>
#include <can/logger.h>

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

void write_header_(struct BLFWriter * blf_writer, FILE * file, uint64_t filesize) {
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
    header.uncompressed_size = blf_writer->uncompressed_size;

    header.object_count = blf_writer->object_count;
    header.count_of_objects_read = 0;

    timestamp_to_systemtime(blf_writer->start_timestamp, header.start_timestamp);
    timestamp_to_systemtime(blf_writer->stop_timestamp, header.stop_timestamp);

    fwrite(&header, sizeof(struct Header), 1, file);

    uint16_t pad_size = (FILE_HEADER_SIZE - sizeof (header)) / 8;
    uint8_t padding[8] = {0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0};
    for (int i=0; i<pad_size; i++)
        fwrite(&padding, sizeof (uint8_t), 8, file);
}

void flush_(struct BLFWriter * blf_writer, FILE * file) {
    ssize_t obj_header_size = sizeof (struct Obj_Header_Base);
    ssize_t log_container_size = sizeof (struct Log_Container);

    uint8_t data[MAX_CONTAINER_SIZE];
    uint16_t compression_method;
    if (blf_writer->compression_level == Z_NO_COMPRESSION) {
        memcpy(data, blf_writer->buffer, blf_writer->buffer_size);
        compression_method = Z_NO_COMPRESSION;
    } else {
        compress2(data,
                  (uLongf *) &blf_writer->buffer_size,
                  (const Bytef *) blf_writer->buffer,
                  blf_writer->buffer_size,
                  blf_writer->compression_level);
        compression_method = ZLIB_DEFLATE;
    }
    ssize_t data_size = blf_writer->buffer_size;
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
    fwrite(&base_header, obj_header_size, 1, file);

    struct Log_Container container_header;
    container_header.compression_method = compression_method;
    for (int i=0; i<LC_GAP0; i++)
        container_header.gap_0[i] = 0x0;
    container_header.size_uncompressed = data_size;
    for (int i=0; i<LC_GAP1; i++)
        container_header.gap_1[i] = 0x0;
    fwrite(&container_header, log_container_size, 1, file);

    fwrite(data, sizeof (uint8_t), blf_writer->buffer_size, file);

    // Write padding bytes
    uint8_t gap = 0x0;
    for (int i=0; i<obj_size%4; i++)
        fwrite(&gap, sizeof (uint8_t), 1, file);

    blf_writer->uncompressed_size += obj_size;
    blf_writer->buffer_size = 0;
}

void add_object_(struct BLFWriter * blf_writer, FILE * file, uint32_t obj_type, uint8_t data[], ssize_t data_size, double timestamp) {
    ssize_t obj_header_size = sizeof (struct Obj_Header_Base);
    ssize_t obj_header_v1_size = sizeof (struct Obj_Header_v1_Base);
    ssize_t obj_size = obj_header_size + obj_header_v1_size + data_size;

    if (obj_size + blf_writer->buffer_size > MAX_CONTAINER_SIZE) {
        flush_(blf_writer, file);
    }

    if (blf_writer->start_timestamp == 0.0)
        blf_writer->start_timestamp = timestamp;
    blf_writer->stop_timestamp = timestamp;
    timestamp = (timestamp - blf_writer->start_timestamp) * 1e9;

    struct Obj_Header_Base base_header;
    base_header.signature[0] = 'L';
    base_header.signature[1] = 'O';
    base_header.signature[2] = 'B';
    base_header.signature[3] = 'J';
    base_header.header_size = obj_header_size + obj_header_v1_size;
    base_header.header_version = 1;
    base_header.object_size = obj_size;
    base_header.object_type = obj_type;

    memcpy(&blf_writer->buffer[blf_writer->buffer_size], &base_header, sizeof (base_header));
    blf_writer->buffer_size += sizeof(base_header);

    struct Obj_Header_v1_Base obj_header;
    obj_header.flags = TIME_ONE_NANS;
    obj_header.client_index = 0;
    obj_header.object_version = 0;
    if (timestamp > 0)
        obj_header.timestamp = (uint64_t)timestamp; // max(timestamp, 0)
    else
        obj_header.timestamp = (uint64_t)0;

    memcpy(&blf_writer->buffer[blf_writer->buffer_size], &obj_header, sizeof (obj_header));
    blf_writer->buffer_size += sizeof (obj_header);

    memcpy(&blf_writer->buffer[blf_writer->buffer_size], data, data_size);
    blf_writer->buffer_size += data_size;

    blf_writer->object_count += 1;
}

void blf_create_logger(void * logger_ptr, void * args) {
    struct Logger * logger = (struct Logger *)logger_ptr;
    struct BLFWriterArgs * blf_args = (struct BLFWriterArgs*)args;

    logger->file = fopen(logger->file_name, "wb");

    logger->writer = malloc(sizeof(struct BLFWriter));
    struct BLFWriter * blf_writer = (struct BLFWriter*)logger->writer;

    blf_writer->compression_level = blf_args->compression_level;
    blf_writer->buffer_size = 0;
    blf_writer->start_timestamp = 0.0;
    blf_writer->stop_timestamp = 0.0;

    blf_writer->uncompressed_size = FILE_HEADER_SIZE;
    blf_writer->object_count = 0;

    write_header_(blf_writer, logger->file, FILE_HEADER_SIZE);
}

void blf_on_message_received(void * logger_ptr, struct Message * can_msg) {
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

    struct Logger * logger = (struct Logger *)logger_ptr;
    add_object_(
            logger->writer,
            logger->file,
            BLF_CAN_MESSAGE,
            can_msg_buffer,
            sizeof (struct CANMessage),
            can_msg->timestamp);
}

void blf_rollover(void * logger_ptr, const uint64_t filesize, const char * new_filename) {
    struct Logger * logger = (struct Logger *)logger_ptr;

    flush_(logger->writer, logger->file);

    fseek(logger->file, 0, SEEK_SET);
    write_header_(logger->writer, logger->file, filesize);

    fclose(logger->file);

    const char * src = logger->file_name;
    const char * dest = new_filename;
    rename(src, dest);

    free(logger->writer);
    blf_create_logger(logger, logger->file);
}

void blf_stop_logger(void * logger_ptr) {
    struct Logger * logger = (struct Logger *)logger_ptr;

    flush_(logger->writer, logger->file);

    uint64_t filesize = ftell(logger->file);
    fseek(logger->file, 0, SEEK_SET);
    write_header_(logger->writer, logger->file, filesize);

    fclose(logger->file);
    free(logger->writer);
}
