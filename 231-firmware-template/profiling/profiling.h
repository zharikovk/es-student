// profiling.h

#pragma once

#include <stdint.h>
#include <stdbool.h>
#define PROFILING_MAX_STOPWATCHES 10
typedef struct {
    const char *name;
    uint32_t count;
    float mean;      // Арифметическое среднее в мкс
    uint32_t max;    // Максимальное время в мкс
} profiling_result_t;
void profiling_stopwatch_init(uint32_t id, const char *name);
void profiling_start(uint32_t id);
void profiling_stop(uint32_t id);
bool profiling_get(uint32_t id, profiling_result_t *result);
void profiling_reset(void);
