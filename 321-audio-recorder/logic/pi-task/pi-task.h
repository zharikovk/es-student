// pi-task.h

#pragma once

#include <stdint.h>
#include <stdbool.h>

typedef enum {
    PI_STATE_IDLE,
    PI_STATE_RUNNING,
    PI_STATE_READY
} pi_state_t;

void pi_task_init(void);
void pi_task_start(uint32_t n);
void pi_task_handle(void);
pi_state_t pi_task_get_state(void);
bool pi_task_get_result(double *result, uint32_t *time_ms);
