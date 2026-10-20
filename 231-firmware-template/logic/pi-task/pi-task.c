#include "pi-task.h"
#include "systime/systime.h"

#define PI_TERMS_PER_CALL 50


// Состояние расчёта: должно пережить возврат из обработчика
static bool pi_running = false;
static bool pi_ready = false;
static uint32_t pi_k;
static uint32_t pi_n;
static double pi_sum;
static double pi_sign;
static double pi_result;
static uint64_t pi_start_time_ms;
static uint32_t pi_calc_time_ms;

void pi_task_init(void) 
{
    pi_running = false;
    pi_ready = false;
    pi_k = 0;
    pi_n = 0;
    pi_sum = 0.0;
    pi_sign = 1.0;
    pi_result = 0.0;
    pi_calc_time_ms = 0;
}

void pi_task_start(uint32_t n)
{
    pi_n = n;
    pi_k = 0;
    pi_sum = 0.0;
    pi_sign = 1.0;
    pi_ready = false;
    pi_running = true;
    pi_start_time_ms = systime_ms();
}

void pi_task_handle(void)
{
    if (!pi_running) 
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
        pi_running = false;
        pi_ready = true;
        pi_calc_time_ms = (uint32_t)(systime_ms() - pi_start_time_ms);
    }
}

// Готов ли результат: если да, записать его по указателю
bool pi_task_get_result(double *result, uint32_t *time_ms)
{
    if (!pi_ready) 
    {
        return false;
    }
    if (result) *result = pi_result;
    if (time_ms) *time_ms = pi_calc_time_ms;
    return true;
}

pi_state_t pi_task_get_state(void) 
{
    if (pi_running) return PI_STATE_RUNNING;
    if (pi_ready) return PI_STATE_READY;
    return PI_STATE_IDLE;
}
