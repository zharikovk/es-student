// profiling.h

#pragma once

#include "pico/types.h"

void profiling_init(void);// запоминает начальное показание часов
void profiling_iteration(void);//вызывается в начале каждой итерации и учитывает прошлую.
float profiling_avg_us(void);// срденее
uint32_t profiling_max_us(void);//максимум
void profiling_reset_max(void);//сброс
