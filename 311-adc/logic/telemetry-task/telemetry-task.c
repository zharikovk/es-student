#include "telemetry-task.h"
#include "platform/platform.h"
#include "systime/systime.h"
#include <stddef.h>

static telemetry_state_t current_state = TELEMETRY_STATE_IDLE;
static uint32_t period_ms = 0;
static uint64_t last_time_us = 0;
static telemetry_task_callback_t callback = NULL;
// telemetry_task_init(): запомнить функцию для записей, состояние IDLE
void telemetry_task_init(telemetry_task_callback_t on_record) 
{
    callback = on_record;
    current_state = TELEMETRY_STATE_IDLE;
    period_ms = 0;
    last_time_us = 0;
}
// telemetry_task_start(): период меньше 10 мс не принять,
bool telemetry_task_start(uint32_t new_period_ms) 
{// запомнить период, первую запись — на ближайшей итерации, состояние RUN
    if (new_period_ms < 10) 
    {
        return false;
    }
    period_ms = new_period_ms;
    last_time_us = 0; //time_us_64();
    current_state = TELEMETRY_STATE_RUN;
    return true;
}
// telemetry_task_stop(): состояние IDLE
void telemetry_task_stop(void) 
{
    current_state = TELEMETRY_STATE_IDLE;
}

telemetry_state_t telemetry_task_get_state(void) 
{
    return current_state;
}

uint32_t telemetry_task_get_period_ms(void) 
{
    return period_ms;
}
// telemetry_task_handle():
void telemetry_task_handle(void) 
{
    switch (current_state) 
    {
        case TELEMETRY_STATE_IDLE:
            // простой
            break;// если задача не запущена, сразу выход
        case TELEMETRY_STATE_RUN: 
        {
            uint64_t current_time_us = systime_us();
            if (systime_period_elapsed(&last_time_us, period_ms)) 
        	{
            	telemetry_record_t record;
            // записываем текущее время, напряжение и температуру
                record.time_us = systime_us();
                record.voltage_v = platform_voltage_read();
                record.temp_c = platform_temp_read();
                
                // отдаём запись функции, полученной при запуске
                if (callback != NULL) 
                {
                    callback(&record);
                }
            last_time_us = current_time_us;    
            }
            break;
    	}
	}
}
