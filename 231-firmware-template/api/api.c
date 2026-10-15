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
#include "led-task.h"
#include "button-task.h"
#include "pi-task.h"

// обработчик команды: получает имя и аргументы
typedef void (*api_callback_t)(const command_t *command);
// Структура одной строки в таблице команд
// команда прибора: имя, справка для help и обработчик
typedef struct
{
    const char *name;
    api_callback_t callback;
    const char *help;
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
static void cmd_pi_start(const command_t *command) 
{
    uint32_t terms=100000;
    uint64_t start_us = time_us_64();
    if (command->argc > 1) 
    {
        printf("error: usage pi_start [terms]\n");
        return;
    }
    if (command->argc == 1) 
    {
        if (!parse_u32(command->argv[0], &terms) || terms == 0) 
        {
            printf("error: usage pi_start [terms]\n");
            return;
        }
    }   
    if (pi_task_get_state() == PI_STATE_RUNNING) 
    {
        printf("error: pi is busy\r\n");
        return;
    }
    pi_task_start(terms);
    printf("pi: running\n");
}
///3. Замерить расчёт
///Время расчёта измеряется разностью двух показаний часов. Первое снимается перед расчётом, второе сразу ///после него. Добавьте в main.c обработчик команды://
//volatile double pi_result;

static void cmd_pi(const command_t *command)
{
    pi_state_t state = pi_task_get_state();

    if (state == PI_STATE_RUNNING) 
    {
        printf("pi: running\r\n");
    } else if (state == PI_STATE_READY) 
    {
        double result;
        uint32_t time_ms;
        if (pi_task_get_result(&result, &time_ms)) 
        {
            printf("pi: %.8f, time: %lu ms\n", result, (unsigned) time_ms);
        }
    } else 
    {
        printf("pi: not started\r\n");
    }
}
//////////////////////////////////////////////////

static void cmd_help(const command_t *command) 
{

    extern const api_command_t command_table[];
    extern const int command_table_size;
    
    for (uint32_t i = 0; i < command_table_size; i++) 
    {
        printf("  %-15s %s\r\n", command_table[i].name, command_table[i].help);
    }
}

static void cmd_led_enable(const command_t *command)
{
	led_task_set_state(LED_STATE_ON);
    printf("led: on, period %lu ms\r\n", (unsigned)led_task_get_period_ms());
}

static void cmd_led_disable(const command_t *command) 
{
    led_task_set_state(LED_STATE_OFF);
    printf("led: off, period %lu ms\r\n", (unsigned)led_task_get_period_ms());
}

static void cmd_led_blink(const command_t *command) 
{
    led_task_set_state(LED_STATE_BLINK);
    printf("led: blink, period %lu ms\r\n", (unsigned)led_task_get_period_ms());
}

///////////////////////////
static void cmd_led_period(const command_t *command)
{
    uint32_t period_ms;

    if (command->argc != 1 || !parse_u32(command->argv[0], &period_ms))
    {
        printf("error: usage led_period <period_ms>\n");
        return;
    }

    // годится ли такой период, решает задача светодиода
    if (!led_task_set_period_ms(period_ms))
    {
        printf("error: period_ms must be greater than 0\n");
        return;
    }

    // ответ: состояние и период светодиода
    printf("led: period %lu ms\n", (unsigned)period_ms);
}
//////////////////////////
static void cmd_button(const command_t *command) 
{
    bool pressed = button_task_is_pressed();
    uint32_t count = button_task_get_press_count();
    
    if (pressed) 
    {
        printf("button: pressed, presses %lu\n", (unsigned)count);
    } else 
    {
        printf("button: released, presses %lu\n", (unsigned)count);
    }
}

///////////////////////////
/////////////////////////
///////////// /////////////
////////// ///////////////
/////// /////////////////
const api_command_t command_table[] = 
{
    {"help",  cmd_help,  "list of commands"},
    {"info",  cmd_info,  "device passport"},
    {"uptime", cmd_uptime, "time since reset"},
	{"pi_start", cmd_pi_start, "calculate pi: pi_start [terms]"},
	{"pi", cmd_pi, "pi calculation result or 'busy'"},
	{"main_time_exec", main_time_exec, "superloop iteration: average and maximum"},
	{"main_time_reset", main_time_reset, "reset maximum iteration"},
    {"led_enable",  cmd_led_enable,  "turn LED on"},
    {"led_disable",  cmd_led_disable,  "turn LED off"},
    {"led_blink",  cmd_led_blink,  "start LED blinkin"},
    {"led_period",  cmd_led_period,  "set blink period in ms: led_period <ms>"},
    {"button",  cmd_button,  "show button state and press count"},
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
