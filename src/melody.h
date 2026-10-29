#ifndef MELODY_H
#define MELODY_H

#include "errors.h"
#include <stdint.h>

enum melody_id { NONE, MELODY_GAMA };

struct melody_handle;

typedef struct melody_handle* melody_handle_t;

operation_result_t get_melody_by_id(enum melody_id id, melody_handle_t* handle);
operation_result_t get_next_note_params(melody_handle_t melody, uint32_t* frequency, uint32_t* duration);
operation_result_t release_melody(melody_handle_t handle);

#endif // MELODY_H