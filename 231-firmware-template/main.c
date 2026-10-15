//main.c
#include "pico/stdlib.h"// добавляем заголовочный файл функций ввода-вывода
#include <stdio.h>// добавляем заголовочные файлы SDK
#include <string.h>// добавляем заголовочные файлы SDK

#include "log.h" // модуль журнала
#include "api.h"
#include "stdio-text-protocol.h"
#include "profiling.h"
#include "led-task.h"
#include "button-task.h"
#include "pi-task.h"

typedef enum {
    STOPWATCH_LOOP,
    STOPWATCH_COMMAND,
    STOPWATCH_LED,
    STOPWATCH_BUTTON,
    STOPWATCH_PI,
    STOPWATCH_COUNT
} stopwatch_id_t;

int main() //inlet)
{
    stdio_init_all();
    sleep_ms(1500);
    ////////////////////////Ф-ия версии
// печатаем имя, версию, дату и время сборки, текущий уровень
    LOG_INF("device: %s %s\n", DEVICE_NAME, FIRMWARE_VERSION);
    stdio_text_protocol_init();
    stdio_init_all();// включаем стандартный ввод-вывод
    // весь дальнейший код пишем здесь
    led_task_init();
    //profiling_init(); // запоминаем начальное показание часов для профилирования
    button_task_init();
    profiling_stopwatch_init(STOPWATCH_LOOP, "loop");
    profiling_stopwatch_init(STOPWATCH_COMMAND, "command");
    profiling_stopwatch_init(STOPWATCH_LED, "led");
    profiling_stopwatch_init(STOPWATCH_BUTTON, "button");
    profiling_stopwatch_init(STOPWATCH_PI, "pi");
        
    while (1)
	{
        profiling_start(STOPWATCH_LOOP);

        profiling_start(STOPWATCH_COMMAND);
        const command_t *command = stdio_text_protocol_handle();
        if (command != NULL) {
            api_handle(command);
        }
        profiling_stop(STOPWATCH_COMMAND);

        profiling_start(STOPWATCH_LED);
	    led_task_handle();// мигаем светодиодом
        profiling_stop(STOPWATCH_LED);

        profiling_start(STOPWATCH_BUTTON);
        button_task_handle();//кнопка
        profiling_stop(STOPWATCH_BUTTON);

        profiling_start(STOPWATCH_PI);
        pi_task_handle();///расчет пи
        profiling_stop(STOPWATCH_PI);

        profiling_stop(STOPWATCH_LOOP);
    }
}


