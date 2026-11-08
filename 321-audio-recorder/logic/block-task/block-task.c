//block-task.c
#include "block-task.h"
#include "platform/platform.h"
#include <stddef.h>

// Состояние задачи захвата блока
static bool is_running = false;
static uint32_t capture_count = 0;
static block_task_callback_t user_callback = NULL;
// block_task_start(): захват уже идёт или platform его не начал — false;
// запомнить число отсчётов и функцию
bool block_task_start(uint32_t count, block_task_callback_t on_block)
{
    //если захват уже идёт или не передана функция-обработчик — отказываем
    if (is_running || on_block == NULL) 
    {
        return false;
    }
    //пытаемся запустить захват на уровне platform
    if (!platform_block_start(count)) 
    {
        return false; // АЦП занят или count слишком большой
    }

    // запоминаем параметры и меняем состояние
    capture_count = count;
    user_callback = on_block;
    is_running = true;

    return true;
}
// block_task_handle(): функции нет или блок не готов — выйти;
// забыть функцию, затем вызвать её с отсчётами, числом и временем
void block_task_handle(void)
{
    // если захват не запущен или нет обработчика, выходить сразу
    if (!is_running || user_callback == NULL) 
    {
        return;
    }
    //проверяем, готов ли блок
    if (!platform_block_done()) 
    {
        return; 
    }
    block_task_callback_t cb = user_callback;
    uint32_t count = capture_count;

    // сбрасываем состояние, разрешая новый запуск
    user_callback = NULL;
    is_running = false;

    //вызываем обработчик с данными из platform
    cb(platform_block_samples(), count, platform_block_time_us());
}
