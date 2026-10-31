// api-callbacks.c
#include "api.h"
#include "api-commands.h"
#include "device/device.h" // модуль железа
#include "firmware.h"
#include "platform/platform.h"
#include "systime/systime.h"
#include "profiling/profiling.h"
#include "stdio-text-protocol/stdio-text-protocol.h"
#include "led-task.h"
#include "button-task.h"
#include "pi-task.h"
#include "telemetry-task.h"
#include <ctype.h>
#include <errno.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
// период телеметрии по умолчанию
#define TELEMETRY_DEFAULT_PERIOD_MS 100

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

static void cmd_charge_on(const command_t *command) 
{
    platform_charge_set(true);
    printf("charge: on\n");
}

static void cmd_charge_off(const command_t *command) 
{
    platform_charge_set(false);
    printf("charge: off\n");
}


// api_print_telemetry(): 
void api_print_telemetry(const telemetry_record_t *record) 
{
    // строка tm: <время> <напряжение %.3f> <температура %.1f>
    printf("tm: %llu %.3f %.1f\n", 
           (unsigned long long)record->time_us, 
           record->voltage_v, 
           record->temp_c);
}
static void cmd_tm_start(const command_t *command) 
{
    uint32_t period = TELEMETRY_DEFAULT_PERIOD_MS;
    if (command->argc>1) 
    {
	    printf("error: usage tm_start [<period_ms>]\n");
        return;
	}
    if (command->argc == 1)
    {
        if (!parse_u32(command->argv[0], &period))  
        {
            printf("error: usage tm_start [<period_ms>]\n");
            return;
        }
    }
    if (telemetry_task_start(period)) 
    {
        printf("telemetry: started, period %lu ms\n", (unsigned long)period);
    } else 
    {
        printf("error: period_ms must be at least 10\n");
    }
}

static void cmd_tm_stop(const command_t *command) 
{
    telemetry_task_stop();
    printf("telemetry: stopped\n");
}

static void cmd_get_temp(const command_t *command)
{
	float temp = platform_temp_read();
	printf("temp: %.1f C\n", temp);
    return;	
}	

static void cmd_get_voltage(const command_t *command)
{
	float voltage = platform_voltage_read();
	printf("voltage: %.3f V\n", voltage);
    return;	
}	






static void cmd_info(const command_t *command) 
{
    // Паспорт устройства 
	device_info_t info;
    device_get_info(&info, FIRMWARE_PROJECT, FIRMWARE_REPO, FIRMWARE_BOARD);
    printf("name: %s\n", FIRMWARE_NAME);
    printf("version: %s\n", FIRMWARE_VERSION);
   	printf("project: %s\n", info.project);
    printf("repo: %s\n", info.repo);
    printf("board: %s\n", info.board);
    printf("serial: %s\n", info.serial);
    printf("chip: manufacturer 0x%03lx, part 0x%04lx, revision %lu\n", 
           (unsigned long)info.chip_manufacturer,
           (unsigned long)info.chip_part,
           (unsigned long)info.chip_revision);
    printf("pico-sdk: %s\n", info.pico_sdk_version);
}

static void cmd_uptime(const command_t *command)
{
    // Время в миллисекундах с момента включения
    printf("uptime: %lu ms\n", (unsigned long)systime_us() / 1000);
}

static void cmd_pi_start(const command_t *command) 
{
    uint32_t terms=1000000;
    if (command->argc > 1) 
    {
        printf("error: usage pi_start [<terms>]\n");
        return;
    }
    if (command->argc == 1) 
    {
        if (!parse_u32(command->argv[0], &terms)) 
        {
            printf("error: usage pi_start [<terms>]\n");
            return;
        }
    }
    if (terms == 0) 
    {
        printf("error: terms must be greater than 0\n");
        return;
    }  
    if (pi_task_get_state() == PI_STATE_RUNNING) 
    {
        printf("error: pi is busy\n");
        return;
    }
    pi_task_start(terms);
    printf("pi: started, %lu terms\n", (unsigned long)terms);
}

static void cmd_pi(const command_t *command)
{
    pi_state_t state = pi_task_get_state();

    if (state == PI_STATE_RUNNING) 
    {
        printf("pi: running\n");
    } else if (state == PI_STATE_READY) 
    {
        double result;
        uint32_t time_ms;
        if (pi_task_get_result(&result, &time_ms)) 
        {
            printf("pi: %.9f in %lu ms\n", result, (unsigned long) time_ms);
        }
    } else 
    {
        printf("pi: not started\n");
    }
}
//////////////////////////////////////////////////

static void cmd_profiling(const command_t *command) 
{
#if PROFILING_ENABLED
    profiling_result_t res;
    // выборка команд в main.c
    for (uint32_t i = 0; i < PROFILING_MAX_STOPWATCHES; i++) 
    {
        if (profiling_get(i, &res)) 
        {
            printf("%-11s count %-11lu mean %.2f us  max %lu us\r\n", 
                   res.name, (unsigned long)res.count, res.mean, (unsigned long)res.max);
        }
    }
#else
    printf("profiling is disabled in this build\r\n");
#endif
}
static void cmd_profiling_reset(const command_t *command) 
{
    profiling_reset();
    printf("profiling reset\r\n");
}
// Число без знака из десятичной записи. false — в записи не только цифры
// или число не помещается в uint32_t.


//////////////////////////////////

///3. Замерить расчёт
///Время расчёта измеряется разностью двух показаний часов. Первое снимается перед расчётом, второе сразу ///после него. Добавьте в main.c обработчик команды://
//volatile double pi_result;

static void cmd_led_enable(const command_t *command)
{
	led_task_set_state(LED_STATE_ON);
    printf("led: on, period %lu ms\r\n", (unsigned long)led_task_get_period_ms());
}

static void cmd_led_disable(const command_t *command) 
{
    led_task_set_state(LED_STATE_OFF);
    printf("led: off, period %lu ms\r\n", (unsigned long)led_task_get_period_ms());
}

static void cmd_led_blink(const command_t *command) 
{
    led_task_set_state(LED_STATE_BLINK);
    printf("led: blink, period %lu ms\r\n", (unsigned long)led_task_get_period_ms());
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
    led_state_t state = led_task_get_state();
    if (state == LED_STATE_OFF) 
    {
        printf("led: off, period %lu ms\n", (unsigned long)period_ms);
    } else if (state == LED_STATE_ON) 
    {
        printf("led: on, period %lu ms\n", (unsigned long)period_ms);
    } else 
    {
        printf("led: blink, period %lu ms\n", (unsigned long)period_ms);
    }
}
//////////////////////////
static void cmd_button(const command_t *command) 
{
    bool is_pressed = button_task_is_pressed();
    uint32_t count = button_task_get_press_count();
    
    if (is_pressed) 
    {
        printf("button: pressed, presses %lu\n", (unsigned long)count);
    } else 
    {
        printf("button: released, presses %lu\n", (unsigned long)count);
    }
}

void cmd_help(const command_t *command) 
{
    for (uint32_t i = 0; i < api_command_count; i++) 
    {
        printf("%-16s %s\n", api_commands[i].name, api_commands[i].help_text);
    }
}
const api_command_t api_commands[] = 
{
    {"help",  cmd_help,  "list of commands"},
    {"info",  cmd_info,  "device passport"},
    {"uptime", cmd_uptime, "time since reset"},
	{"pi_start", cmd_pi_start, "calculate pi: pi_start [terms]"},
	{"pi", cmd_pi, "pi calculation result or 'busy'"},
	{"profiling", cmd_profiling, "superloop iteration: average and maximum"},
	{"profiling_reset", cmd_profiling_reset, "reset profiling"},
    {"led_enable",  cmd_led_enable,  "turn LED on"},
    {"led_disable",  cmd_led_disable,  "turn LED off"},
    {"led_blink",  cmd_led_blink,  "start LED blinkin"},
    {"led_period",  cmd_led_period,  "set blink period in ms: led_period <ms>"},
    {"button",  cmd_button,  "show button state and press count"},
    { "get_voltage", cmd_get_voltage, "voltage on GP26, ADC input 0"},
    { "get_temp", cmd_get_temp, "chip temperature, degrees Celsius"},
    { "tm_start", cmd_tm_start, "start telemetry with optional period in ms (default 100)" },
    { "tm_stop", cmd_tm_stop, "stop telemetry" },
    { "charge_on",  cmd_charge_on,  "turn on RC circuit charge (GP22 high)" },
    { "charge_off", cmd_charge_off, "turn off RC circuit charge (GP22 low)" }
};

const uint32_t api_command_count = sizeof(api_commands) / sizeof(api_commands[0]);
