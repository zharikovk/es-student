// api.c
#include "device.h" // модуль железа
#include "api.h"
#include <stdio.h>
#include <string.h>
#include "pico/stdlib.h" 
#include "profiling.h"
#include "stdio-text-protocol.h"
#include <ctype.h>
#include <errno.h>
#include <stdlib.h>

// обработчик команды: получает имя и аргументы
typedef void (*api_callback_t)(const command_t *command);
// Структура одной строки в таблице команд
// команда прибора: имя, справка для help и обработчик
typedef struct
{
    const char *name;
    const char *help;
    api_callback_t callback;
} api_command_t;

static void cmd_info(const command_t *command) 
{
    // Паспорт устройства 
    device_info();
}

static void cmd_uptime(const command_t *command)
{
    // Время в миллисекундах с момента включения
    uint64_t uptime_ms = time_us_64() / 1000;
    printf("uptime: %llu ms\n", time_us_64() / 1000);
}


static void main_time_exec(const command_t *command)
{
	printf("iteration avg %.2f us, max %u us\n", profiling_avg_us(), (unsigned)profiling_max_us());
}

static void main_time_reset(const command_t *command)
{
	profiling_reset_max();
	printf("max reset\n");
}

// Число без знака из десятичной записи. false — в записи не только цифры
// или число не помещается в uint32_t.
static bool parse_u32(const char *text, uint32_t *value)
{
    // strtoul приняла бы и знак: из "-1" получилось бы 4294967295
    if (!isdigit((unsigned char)text[0]))
    {
        return false;
    }

    char *end;
    errno = 0;
    unsigned long number = strtoul(text, &end, 10);

    // после числа остались символы или число не поместилось
    if (*end != '\0' || errno == ERANGE || number > UINT32_MAX)
    {
        return false;
    }

    *value = (uint32_t)number;
    return true;
}

//////////////////////////////////
static double calc_pi(uint terms)
{
    double sum = 0.0;
    double sign = 1.0;

    for (int k = 0; k < terms; k++) 
    {
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

static void cmd_calc_pi(const command_t *command)
{
    uint32_t terms;
    uint64_t start_us = time_us_64();
    if (command->argc > 1) 
    {
        // Слишком много аргументов
        printf("error: usage calc_pi [terms]\n");
        return;
    }
    if (command->argc == 1) 
    {
        if (!parse_u32(command->argv[0], &terms)) 
        {
            // Не удалось разобрать как число
            printf("error: usage calc_pi [terms]\n");
            return;
        }
        // 3. Проверка на ноль
        if (terms == 0) 
        {
            printf("error: terms must be positive\n");
            return;
        }
    }
    pi_result = calc_pi(terms);
    uint64_t spent_us = time_us_64() - start_us;

    printf("pi: %.8f\n", pi_result);
    printf("time: %llu ms\n", spent_us / 1000);
}



static void cmd_help(const command_t *command) 
{

    extern const api_command_t command_table[];
    extern const int command_table_size;
    
    for (uint32_t i = 0; i < command_table_size; i++) 
    {
        printf("  %-15s %s\r\n", command_table[i].name, command_table[i].help);
    }
}

const api_command_t command_table[] = 
{
    {"help",  cmd_help,  "list of commands"},
    {"info",  cmd_info,  "device passport"},
    {"uptime", cmd_uptime, "time since reset"},
	{"calc_pi", cmd_calc_pi, "calculate pi: calc_pi [terms]"},
	{"main_time_exec", main_time_exec, "superloop iteration: average and maximum"},
	{"main_time_reset", main_time_reset, "reset maximum iteration"},
    
};

const int command_table_size = sizeof(command_table) / sizeof(command_table[0]);

void api_init(void) 
{
    // Пока ничего 
}

void api_handle(const command_t *command) 
{
    if (command->truncated) 
    {
        printf("error: command longer than 63 characters or 4 arguments\r\n");
        return;
    }

    if (command->name == 0) 
    {
        return;
    }


    for (uint32_t i = 0; i < command_table_size; i++) 
    {
        if (strcmp(command->name, command_table[i].name) == 0) 
        {
            command_table[i].callback(command);
            return;
        }
    }
    printf("error: unknown command '%s', try help\n", command->name);
}
