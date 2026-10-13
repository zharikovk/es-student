#include "pico/stdlib.h"// добавляем заголовочный файл функций ввода-вывода
#include <stdio.h>// добавляем заголовочные файлы SDK
#include <string.h>// добавляем заголовочные файлы SDK

#include "log.h" // модуль журнала
#include "api.h"
#include "stdio-text-protocol.h"
#include "profiling.h"
#include "led-task.h"


int main() //inlet)
{
    stdio_text_protocol_init();
    stdio_init_all();// включаем стандартный ввод-вывод
    // весь дальнейший код пишем здесь
    led_task_init();
    profiling_init(); // запоминаем начальное показание часов для профилирования
    
    while (1)
	{
	    profiling_iteration();// учитываем прошлую итерацию
	    led_task_handle();// мигаем светодиодом
        
        const command_t *command = stdio_text_protocol_handle();
        if (command != NULL) {
            api_handle(command);
        }
        //sleep_ms(10);	    
	}
}


