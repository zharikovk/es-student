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
// по умолчанию профилирование включено
#ifndef PROFILING_ENABLED
#define PROFILING_ENABLED 1
#endif

void profiling_stopwatch_init(uint32_t id, const char *name);
void profiling_reset(void);
bool profiling_get(uint32_t id, profiling_result_t *result);

#if PROFILING_ENABLED
void profiling_start(uint32_t id);
void profiling_stop(uint32_t id);
#else
// профилирование выключено: вызовы ничего не делают
#define profiling_start(id) ((void)0)
#define profiling_stop(id) ((void)0)
#endif
