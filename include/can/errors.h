#ifndef C_CAN_ERRORS_H
#define C_CAN_ERRORS_H

typedef enum {
    SUCCESS = 0,
    INVALID_PARAMETERS,
    OUT_OF_MEMORY,
    FAILED_OPERATION,
    NOT_SUPPORTED,
    FILE_NOT_FOUND,
    GENERIC_ERROR
} c_can_err_t;

#endif //C_CAN_ERRORS_H
