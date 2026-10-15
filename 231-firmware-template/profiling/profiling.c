// profiling.c

#include "pico/stdlib.h"
#include "profiling.h"

typedef struct 
{
    const char *name;
    bool is_active;
    uint32_t count;
    uint32_t sum_us;
    uint32_t max_us;
    uint32_t start_us;
} stopwatch_t;

static stopwatch_t stopwatches[PROFILING_MAX_STOPWATCHES];
void profiling_stopwatch_init(uint32_t id, const char *name) 
{
    if (id >= PROFILING_MAX_STOPWATCHES) return;
    
    stopwatches[id].name = name;
    stopwatches[id].is_active = true;
    stopwatches[id].count = 0;
    stopwatches[id].sum_us = 0;
    stopwatches[id].max_us = 0;
    stopwatches[id].start_us = 0;
}

void profiling_start(uint32_t id) 
{
    if (id >= PROFILING_MAX_STOPWATCHES || !stopwatches[id].is_active) return;
    stopwatches[id].start_us = time_us_32();
}

void profiling_stop(uint32_t id) {
    if (id >= PROFILING_MAX_STOPWATCHES || !stopwatches[id].is_active) return;
    
    uint32_t now_us = time_us_32();
    uint32_t elapsed_us = now_us - stopwatches[id].start_us;
    
    stopwatches[id].count++;
    stopwatches[id].sum_us += elapsed_us;
    
    if (elapsed_us > stopwatches[id].max_us) 
    {
        stopwatches[id].max_us = elapsed_us;
    }
}

bool profiling_get(uint32_t id, profiling_result_t *result) 
{
    if (id >= PROFILING_MAX_STOPWATCHES || !stopwatches[id].is_active || result == NULL) 
    {
        return false;
    }
    
    result->name = stopwatches[id].name;
    result->count = stopwatches[id].count;
    result->max = stopwatches[id].max_us;
    
    // Арифметическое среднее. Защита от деления на ноль.
    if (stopwatches[id].count > 0) 
    {
        result->mean = (float)stopwatches[id].sum_us / stopwatches[id].count;
    } else 
    {
        result->mean = 0.0f;
    }
    
    return true;
}

void profiling_reset(void) 
{
    for (uint32_t i = 0; i < PROFILING_MAX_STOPWATCHES; i++) 
    {
        if (stopwatches[i].is_active) 
        {
            stopwatches[i].count = 0;
            stopwatches[i].sum_us = 0;
            stopwatches[i].max_us = 0;
        }
    }
}
