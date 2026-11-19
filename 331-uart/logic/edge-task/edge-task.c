// edge-task.c

#include "edge-task.h"
#include "platform/platform.h"
#include "systime/systime.h"


// edge-task.c
// пороги EMPTY_V 0.05f, RISE_V 2.0f, FULL_V 3.2f, FALL_V 1.3f, TIMEOUT_US 2000000
// состояние, режим линии, функция результата, начало фазы, время фронта вверх
// Пороговые значения и таймаут из задания
#define EMPTY_V    0.1f
#define RISE_V     2.0f
#define FULL_V     3.2f
#define FALL_V     1.3f
#define TIMEOUT_US 2000000

typedef enum 
{
    STATE_IDLE,
    STATE_DISCHARGE,
    STATE_RISE,
    STATE_CHARGE,
    STATE_FALL
} edge_state_t;
// enter(): уровень на линии через platform_line_set(), начало фазы — сейчас, новое состояние
// finish(): состояние IDLE, затем вызов функции результата
static edge_state_t state = STATE_IDLE;
static platform_line_mode_t current_mode;
static edge_task_callback_t callback;
static uint32_t phase_start_us;
static uint32_t rise_us;

static void finish(bool ok, uint32_t rise, uint32_t fall) 
{
       // printf("DEBUG: finish() вызвана! ok=%d, rise=%lu, fall=%lu\r\n", ok, (unsigned long)rise, (unsigned long)fall);

    platform_line_set(current_mode, false);//разрядка
    state = STATE_IDLE;
    if (callback) 
    {
        callback(current_mode, ok, rise, fall);
    }
}
// edge_task_start(): замер уже идёт — false; иначе запомнить режим и функцию,
// ноль на линии, состояние DISCHARGE
bool edge_task_start(platform_line_mode_t mode, edge_task_callback_t on_done) 
{
    
 /*       static uint32_t dbg_counter = 0;
    if (++dbg_counter % 1000 == 0) {
        printf("DBG HANDLE: state=%d\r\n", state);
    }*/
    
    // замер уже идёт
    if (state != STATE_IDLE) 
    {
        return false;
    }
    
    // запоминаем режим и функцию
    current_mode = mode;
    callback = on_done;
    
    // ноль на линии, начало фазы — сейчас, новое состояние DISCHARGE
    platform_line_set(current_mode, 0); // или false, в зависимости от сигнатуры в platform.h
    phase_start_us = systime_us();
    state = STATE_DISCHARGE;
    
    return true;
}
// edge_task_handle(): в IDLE ничего; иначе прошедшее время фазы и напряжение узла,
// переходы по таблице, фаза дольше TIMEOUT_US — finish(false, 0)
void edge_task_handle(void) 
{
    // в IDLE ничего не делаем
    if (state == STATE_IDLE) 
	return;
    uint32_t now_us = systime_us();
    uint32_t elapsed_us = now_us - phase_start_us;
    
    if (elapsed_us > TIMEOUT_US) 
    {
//		printf("DEBUG TIMEOUT: state=%d, last_V=%.3f\r\n", state, platform_voltage_read());
        finish(false, 0, 0);
        return;
    }

    float voltage = platform_voltage_read();

/*   static uint32_t last_dbg = 0;
    if (now_us - last_dbg > 100000) { // 100 000 мкс = 100 мс
        printf("DBG: state=%d, V=%.3f\r\n", state, voltage);
        printf("YA_IZMENIL_FAYL!!! state=%d, V=%.3f\r\n", state, voltage);
        last_dbg = now_us;
    }
*/
    // переходы по таблице состояний
   switch (state) 
    {
        case STATE_DISCHARGE:
            if (voltage <= EMPTY_V) 
            {
//                printf("DEBUG SWITCH: Переход DISCHARGE -> RISE. Вызываю platform_line_set(mode=%d, high=1)\r\n", current_mode);
                platform_line_set(current_mode, true); // Поднимаем линию
                phase_start_us = now_us;               // Начало отсчета фронта вверх
                state = STATE_RISE;
            }
            break;

        case STATE_RISE:
            if (voltage >= RISE_V) 
            {
                rise_us = elapsed_us;      // Засекли время достижения порога
                state = STATE_CHARGE;      // Переходим к фазе полного заряда
            }
            if (voltage >= FULL_V) 
            {
//                printf("DEBUG SWITCH: Переход RISE -> FALL. Вызываю platform_line_set(mode=%d, high=0)\r\n", current_mode);
                platform_line_set(current_mode, false); // Опускаем линию для замера спада
                phase_start_us = now_us;                // Сброс начала фазы для спада
                state = STATE_FALL;
            }
            break;

        case STATE_CHARGE:
            if (voltage >= FULL_V) 
            {
//                printf("DEBUG SWITCH: Переход CHARGE -> FALL. Вызываю platform_line_set(mode=%d, high=0)\r\n", current_mode);
                platform_line_set(current_mode, false); // Опускаем линию для замера спада
                phase_start_us = now_us;                // Сброс начала фазы для спада
                state = STATE_FALL;
            }
            break;

        case STATE_FALL:
            if (voltage <= FALL_V) 
            {
                finish(true, rise_us, elapsed_us);      // Успех!
            }
            break;

        default:
            state = STATE_IDLE;
            break;
    }
}
