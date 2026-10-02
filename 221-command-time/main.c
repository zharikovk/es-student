#include "pico/stdlib.h"// добавляем заголовочный файл функций ввода-вывода
#include <stdio.h>// добавляем заголовочные файлы SDK
#include <string.h>// добавляем заголовочные файлы SDK
#include "led.h" // модуль Светодиод
#include "log.h" // модуль журнала
#include "device.h" // модуль железа
#include "memory.h"
#include "command.h"
#include "clock.h"

#define LINE_SIZE 32//буфер и его текущую длин


extern void boot_info(void);
char line[LINE_SIZE];//буфер и его текущую длин
uint line_length = 0;//буфер и его текущую длин

void cmd_version(void)
{
        log_version();
}
void cmd_info(void)
{
        device_info();
}

void cmd_ping(void)
{
    printf("pong\n");
}

void cmd_mem_info(void)
{
    mem_info();
}

void cmd_clk_info(void)
{
    clk_info();
}

void cmd_uptime(void)
{
	uptime();
}

////////////////////////////////////////////
////по команде calc_pi прибор считает число π рядом Лейбница////
// прикидка: за член ряда 4 операции с double, 175 + 110 + 190 + 110 = 585 тактов;
// 1 000 000 членов по 585 тактов при 125 МГц — около 4,7 с
const uint CALC_PI_TERMS = 1000000;
//////////////////////////////////
double calc_pi(uint terms)
{
    double sum = 0.0;
    double sign = 1.0;

    for (int k = 0; k < terms; k++) {
        // Прибавить очередной член ряда: ±1 / (2k + 1)
        sum += sign / (2.0 * k + 1.0);
        sign = -sign;
    }

    // Сумма ряда равна π/4
    return 4.0 * sum;
}

///3. Замерить расчёт
///Время расчёта измеряется разностью двух показаний часов. Первое снимается перед расчётом, второе сразу ///после него. Добавьте в main.c обработчик команды://
volatile double pi_result;

void cmd_calc_pi(void)
{
    uint64_t start_us = time_us_64();
    pi_result = calc_pi(CALC_PI_TERMS);
    uint64_t spent_us = time_us_64() - start_us;

    printf("pi: %.8f\n", pi_result);
    printf("time: %llu ms\n", spent_us / 1000);
}

const struct command_t commands[] = {
    { "info", cmd_info },
    { "version", cmd_version },
    { "ping", cmd_ping },
    { "mem_info", cmd_mem_info },
    { "fw_info", fw_info },
    { "dev_info", dev_info},
    { "boot_info", boot_info},
    { "clk_info", cmd_clk_info},
    { "uptime", cmd_uptime},
    { "calc_pi", cmd_calc_pi }
};
const uint command_count = sizeof(commands) / sizeof(commands[0]);

void handle_command(const char *command)
{
    for (uint i = 0; i < command_count; i++)
    {
        if (strcmp(command, commands[i].name) == 0)
        {
            if (commands[i].handler != NULL)
            {
                commands[i].handler();
            }

            return;
        }
    }

    LOG_ERR("unknown command: %s\n", command);
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
    stdio_init_all();// включаем стандартный ввод-вывод
    // весь дальнейший код пишем здесь
    led_init();// инициализируем пин светодиода
     
    while (1)
	{
	    blink();
        read_line();	    
	}
}


