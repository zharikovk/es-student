#include "pi-task.h"
#include "pico/stdlib.h"
#include "systime/systime.h"

#define PI_TERMS_PER_CALL 2000


// Состояние расчёта: должно пережить возврат из обработчика
static pi_state_t pi_state = PI_STATE_NOT_STARTED;
static uint32_t pi_k = 0;
static uint32_t pi_n = 0;
static double pi_sum = 0.0;
static double pi_sign = 1.0;
static double pi_result = 0.0;
static uint32_t calc_time_ms = 0;
static uint64_t start_time_us = 0;

void pi_task_start(uint32_t n)
{
    pi_state = PI_STATE_RUNNING;
    pi_n = n;
    pi_k = 0;
    pi_sum = 0.0;
    pi_sign = 1.0;
    start_time_us = systime_us();
}

void pi_task_handle(void)
{
    if (pi_state != PI_STATE_RUNNING) 
    {
        return;
    }

    // Очередная порция членов ряда, но не дальше последнего
    uint32_t end = pi_k + PI_TERMS_PER_CALL;
    if (end > pi_n) 
    {
        end = pi_n;
    }
    for (; pi_k < end; pi_k++) 
    {
        pi_sum += pi_sign / (2.0 *  (double)pi_k + 1.0);
        pi_sign = -pi_sign;
    }

    if (pi_k == pi_n) 
    {
        pi_result = 4.0 * pi_sum;
        calc_time_ms = (uint32_t)((systime_us() - start_time_us) / 1000);
        pi_state = PI_STATE_READY;
    }
}

// Готов ли результат: если да, записать его по указателю
bool pi_task_get_result(double *result, uint32_t *time_ms)
{
    if (pi_state != PI_STATE_READY) {
        return false;
    }
    *result = pi_result;
    *time_ms = calc_time_ms;
    return true;
}

pi_state_t pi_task_get_state(void) 
{
    return pi_state;
}
