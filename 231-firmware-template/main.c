#include "pico/stdlib.h"// добавляем заголовочный файл функций ввода-вывода
#include <stdio.h>// добавляем заголовочные файлы SDK
#include <string.h>// добавляем заголовочные файлы SDK
#include "led.h" // модуль Светодиод
#include "log.h" // модуль журнала
#include "api.h"
#include "stdio-text-protocol.h"
#include "profiling.h"










////////////////////////////////////////////
////по команде calc_pi прибор считает число π рядом Лейбница////
// прикидка: за член ряда 4 операции с double, 175 + 110 + 190 + 110 = 585 тактов;
// 1 000 000 членов по 585 тактов при 125 МГц — около 4,7 с


///

/// ///////////////////////////////////////////////////////////////////////
const uint BLINK_HALF_PERIOD_MS = 500;
//Добавьте в main.c константу полупериода, метку прошлого переключения и функцию blink():
uint64_t last_toggle_us = 0;

void blink(void)
{
    uint64_t now_us = time_us_64();

    if (now_us - last_toggle_us >= BLINK_HALF_PERIOD_MS * 1000)
    {
        last_toggle_us = now_us;
        led_toggle();
    }
}

int main() //inlet)
{
    stdio_text_protocol_init();
    stdio_init_all();// включаем стандартный ввод-вывод
    // весь дальнейший код пишем здесь
    led_init();// инициализируем пин светодиода
    //
    profiling_init(); // запоминаем начальное показание часов для профилирования
    
    while (1)
	{
	    profiling_iteration();// учитываем прошлую итерацию
	    blink();// мигаем светодиодом
        
        const command_t *command = stdio_text_protocol_handle();
        if (command != NULL) {
            api_handle(command);
        }
        sleep_ms(10);	    
	}
}


