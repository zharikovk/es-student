#include "pico/stdlib.h"// добавляем заголовочный файл функций ввода-вывода
#include "hardware/gpio.h"// добавляем заголовочный файл функций работы с GPIO
#include <stdio.h>// добавляем заголовочные файлы SDK
#include <string.h>// добавляем заголовочные файлы SDK
#include "led.h" // модуль Светодиод
#include "log.h" // модуль журнала
#include "device.h" // модуль железа

#define LINE_SIZE 32//буфер и его текущую длин

char line[LINE_SIZE];//буфер и его текущую длин
uint line_length = 0;//буфер и его текущую длин

const uint BUTTON_PIN = 15; //// объявляем константу вывода светодиода

const uint DEBOUNCE_MS = 20; /// Заведите константу задержки

bool get_button_debounce(uint pin)//Заведите константу задержки и функцию, которая читает вывод устойчиво
{
    bool state = gpio_get(pin);
    sleep_ms(DEBOUNCE_MS);
    return state && gpio_get(pin);
} ///

//Функция получает пришедший символ и текущее состояние светодиода, а возвращает новое.
void handle_command(const char *command)
{
    if (strcmp(command, "enable") == 0)
    {
        led_set(true);
        LOG_INF("led %s\n", led_is_on() ? "on" : "off");
    }
    else if (strcmp(command, "disable") == 0)
    {
        led_set(false);
        LOG_INF("led %s\n", led_is_on() ? "on" : "off");
    }
    else if (strcmp(command, "version") == 0)//тут понятно вывод версии
    {
        log_version();
    }
    else if (strcmp(command, "info") == 0)//тут понятно вывод железа
    {
        device_info();
    }
    else
    {
        LOG_ERR("unknown command: %s\n", command);
    }
}
///
void read_line(void)//////////////Собрать строку из символов
{
    int symbol = getchar_timeout_us(0);

    if (symbol == PICO_ERROR_TIMEOUT)
    {
        return;
    }

    if (symbol == '\r' || symbol == '\n')
    {
        putchar('\n');
        line[line_length] = '\0';

        if (line_length > 0)
        {
            LOG_DBG("got %s\n", line);
            handle_command(line);
        }

        line_length = 0;
        return;
    }

    if (line_length + 1 < LINE_SIZE)
    {
        line[line_length] = (char)symbol;
        line_length = line_length + 1;
        putchar(symbol);
    }
}
/// ///////////////////////////////////////////////////////////////////////
int main() //inlet)
{
    stdio_init_all();// включаем стандартный ввод-вывод
    // весь дальнейший код пишем здесь
    led_init();// инициализируем пин светодиода
    
    gpio_init(BUTTON_PIN);
    gpio_set_dir(BUTTON_PIN, GPIO_IN);
    gpio_pull_up(BUTTON_PIN);

    
    bool led = false;//переменные - помнит светит ли светодиод
    bool previous = false;// перемеенные - что показывает вывод на прошлом шаге
     
    while (1)
	{
	    
	    bool current = get_button_debounce(BUTTON_PIN);//Теперь в суперцикле замените прямое чтение вывода вызовом функции. Больше в программе не меняется ничего:
        if (previous == true && current == false)
        {// если состояние сменилось — переключаем светодиод вызовом set_led
            led_toggle();
            LOG_INF("led %s\n", led_is_on() ? "on" : "off");
        }
        previous = current;// запоминаем текущее состояние пина кнопки, как предыдущее
        
        read_line();	    
	   
    }
}
