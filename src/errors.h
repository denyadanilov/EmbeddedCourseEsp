#ifndef ERRORS_H
#define ERRORS_H

#define ERROR 1
#define OK 0

enum operation_result { OPERATION_RESULT_ERROR = ERROR, OPERATION_RESULT_OK = OK };

typedef enum operation_result operation_result_t;

#endif // ERRORS_H