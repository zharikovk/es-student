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
#include "block-task.h"
#include "stream-task.h"
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
    if (platform_adc_busy()) 
    {
    	printf("error: adc busy\n");
    	return;
    }
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
	if (platform_adc_busy()) 
	{
        printf("error: adc busy\n");
        return;
    }
	float temp = platform_temp_read();
	printf("temp: %.1f C\n", temp);
    return;	
}	

static void cmd_get_voltage(const command_t *command)
{
	if (platform_adc_busy()) 
	{
        printf("error: adc busy\n");
        return;
    }
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
///////////////////////////////////////////////////
static void cmd_timer_blink_start(const command_t *command)
{
uint32_t period_ms;

    if (command->argc != 1 || !parse_u32(command->argv[0], &period_ms))
    {
        printf("error: usage timer_blink_start <period_ms>\n");
        return;
    }
    if (!platform_blink_start(period_ms))
    {
        printf("error: period_ms must be 10..60000\n");
        return;
    }
    led_task_set_state(LED_STATE_OFF);
    printf("timer_blink: started, period %lu ms\n", (unsigned long)period_ms);
}

static void cmd_timer_blink_stop(const command_t *command)
{
  // ответ 
    uint32_t toggles = platform_blink_stop();
    printf("timer_blink: stopped, %lu toggles\n",(unsigned long)toggles);
}
////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////
static void cmd_cap_start(const command_t *command)
{
    uint32_t count, period_us;

    //аргументов не два или не числа — error: usage cap_start <N> <period_us>
    if (command->argc != 2 || !parse_u32(command->argv[0], &count) || !parse_u32(command->argv[1], &period_us))
    {
        printf("error: usage cap_start <N> <period_us>\n");
        return;
    }

    //  N = 0, N > PLATFORM_CAPTURE_MAX или период меньше 10 мкс —
//     error: N must be 1..2000, period_us at least 10
    if (count == 0 || count > PLATFORM_CAPTURE_MAX || period_us < 10)
    {
        printf("error: N must be 1..2000, period_us at least 10\n");
        return;
    }
// N = 0, N > PLATFORM_CAPTURE_MAX или период меньше 10 мкс —
//     error: N must be 1..2000, period_us at least 10
    if (platform_adc_busy() || !platform_capture_start(count, period_us))
    {
        printf("error: adc busy\n");
        return;
    }
// идёт телеметрия или platform не начал захват — error: adc busy
// ответ capture: started, <N> samples, period <период> us
    printf("capture: started, %lu samples, period %lu us\n", (unsigned long)count, (unsigned long)period_us);
}

static void cmd_cap_dump(const command_t *command)//захвата не было — error: no capture
{
    if (command->argc != 0)
    {
        printf("error: usage cap_dump\n");
        return;
    }

    if (!platform_capture_done())
    {
        printf("error: no capture\n");
        return;
    }


    platform_capture_t cap;
    platform_capture_get(&cap);
// строка capture: <N> samples, period <период> us, interval <мин>..<макс> us, handler <доля> %
    printf("capture: %lu samples, period %lu us, interval %lu..%lu us, handler %.1f %%\n",
           (unsigned long)cap.count,
           (unsigned long)cap.period_us,
           (unsigned long)cap.min_interval_us,
           (unsigned long)cap.max_interval_us,
           (double)cap.handler_percent);

    // вывод по 16 штук в строке
    for (uint32_t i = 0; i < cap.count; i++)
    {
        if (i % 16 == 0)
        {
            printf("cap:");
        }
        printf(" %u", cap.samples[i]);
        
        // Перенос строки после каждых 16 элементов или в конце массива
        if (i % 16 == 15 || i == cap.count - 1)
        {
            printf("\n");
        }
    }
}

// print_dma_cap(): наименьший, наибольший и средний код блока;
static void print_dma_cap(const uint16_t *samples, uint32_t count, uint32_t time_us)
{
    uint16_t min_val = samples[0];
    uint16_t max_val = samples[0];
    uint64_t sum = 0;
    // Проход по всем отсчётам для поиска мин, макс и суммы
    for (uint32_t i = 0; i < count; i++) 
    {
        uint16_t val = samples[i];
        if (val < min_val) min_val = val;
        if (val > max_val) max_val = val;
        sum += val;
    }
    // cреднее значение
    float mean = (float)sum / (float)count;
    // расчёт времени — count * 1000000 / PLATFORM_SAMPLE_RATE_HZ, в uint64_t;
    uint64_t expected_us = (uint64_t)count * 1000000ULL / PLATFORM_SAMPLE_RATE_HZ;

    //строка dma_cap: <N> samples in <мкс> us, expected <мкс> us, min <код> max <код> mean <код, 1 знак>
    printf("dma_cap: %lu samples in %lu us, expected %llu us, min %u max %u mean %.1f\n",
           (unsigned long)count,
           (unsigned long)time_us,
           (unsigned long long)expected_us,
           (unsigned int)min_val, 
           (unsigned int)max_val, 
           (double)mean);
}

//// cmd_dma_cap(): аргумент не один или не число — error: usage dma_cap <N>
static void cmd_dma_cap(const command_t *command)
{
    uint32_t count;
    // проверка наличия и корректности аргумента
    if (command->argc != 1 || !parse_u32(command->argv[0], &count))
    {
        printf("error: usage dma_cap <N>\n");
        return;
    }
    // проверка диапазонов N = 0 или больше PLATFORM_BLOCK_MAX — error: N must be 1..8192
    if (count == 0 || count > PLATFORM_BLOCK_MAX)
    {
        printf("error: N must be 1..8192\n");
        return;
    }
// идёт телеметрия или задача не приняла захват — error: adc busy
    if (!block_task_start(count, print_dma_cap))
    {
        printf("error: adc busy\n");
        return;
    }
// иначе захват с функцией print_dma_cap, ответ придёт, когда блок будет готов
}
////////////////////////////////////////////324
#define STREAM_BLINK_PERIOD_MS 250
// колбэк, который вызывается из stream_task_handle, когда буфер готов
void on_stream_frame(const stream_header_t *header, const uint8_t *data, uint32_t bytes)
{
    // отправляем заголовок кадра (6 байт: синхрослово, номер, количество)
    platform_write(header, sizeof(stream_header_t));
// отправляем сами данные (байты или слова, в зависимости от разрядности)
    platform_write(data, bytes);
}

static void cmd_stream_start(const command_t *command)
{
    uint32_t bits = 16; // По умолчанию 16 бит
    // проверка аргументов
    if (command->argc > 1)
    {
        printf("error: usage stream_start [8|16]\n");
        return;
    }
    
    if (command->argc == 1 && !parse_u32(command->argv[0], &bits))
    {
        printf("error: usage stream_start [8|16]\n");
        return;
    }
    // Пповерка допустимой разрядности
    if (bits != 8 && bits != 16)
    {
        printf("error: usage stream_start [8|16]\n");
        return;
    }
    // попытка запустить поток
    if (!stream_task_start((uint8_t)bits))
    {
        printf("error: adc busy\n");
        return;
    }
    // ваш ответ
    // запускаем индикатор записи (как в задании 3.2.1)
    platform_blink_start(STREAM_BLINK_PERIOD_MS);
    printf("stream: started, 44100 Hz, %lu bit\n", (unsigned long)bits);
}

static void cmd_stream_stop(const command_t *command)
{
    //команда не принимает аргументов
    if (command->argc != 0)
    {
        printf("error: usage stream_stop\n");
        return;
    }
    // проверка, что поток действительно запущен
    if (stream_task_get_state() != STREAM_STATE_RUN)
    {
        printf("error: not running\n");
        return;
    }
    //останавливаем поток и собираем статистику
    platform_blink_stop();
    stream_task_stop();
    printf("stream: stopped, %lu frames, %lu samples, %lu overruns\n",
           (unsigned long)stream_task_get_frames(),
           (unsigned long)stream_task_get_samples(),
           (unsigned long)stream_task_get_overruns());
}
///////////////////////////////////////////////3/2/5
static void cmd_generate_tone(const command_t *command)
{
    uint32_t freq_hz;
    // gроверка аргументов (должен быть ровно один числовой аргумент)
    if (command->argc != 1 || !parse_u32(command->argv[0], &freq_hz)) 
    {
        printf("error: usage generate_tone <Hz>\n");
        return;
    }
    if (!platform_tone_set(freq_hz)) 
    {
        printf("error: Hz must be 0 or 20..20000\n");
        return;
    }

    if (freq_hz == 0) 
    {
        printf("tone: off\n");
    } else 
    {
        printf("tone: %lu Hz\n", (unsigned long)freq_hz);
    }
}
//////////////////////////////////////////////////
//  Команда для намеренного зависания (проверка работы watchdog)
static void cmd_hang(const command_t *command)
{
    printf("hang: superloop stopped\n");

    
    while (1)
    {
        
    }
}


static void cmd_wdt_enable(const command_t *command)
{
    uint32_t timeout_ms;

    if (command->argc != 1 || !parse_u32(command->argv[0], &timeout_ms))
    {
        printf("error: usage wdt_enable <timeout_ms>\n");
        return;
    }

    if (!platform_watchdog_enable(timeout_ms))
    {
        printf("error: timeout_ms must be 1..8388\n");
        return;
    }

    printf("wdt: enabled, timeout %lu ms\n", (unsigned long)timeout_ms);
}


static void cmd_reset_reason(const command_t *command)
{
    uint32_t restarts = 0;
    bool is_wdt = platform_watchdog_caused_reset(&restarts);

    if (is_wdt)
    {
        printf("reset: watchdog, restarts %lu\n", (unsigned long)restarts);
    }
    else
    {
        printf("reset: other, restarts %lu\n", (unsigned long)restarts);
    }
}
/////////////////////////////////////////////////////
static void cmd_version(const command_t *command)
{
    printf("version: 1.0.0\n");
}

const api_command_t api_commands[] = {
    {"help", cmd_help, "list of commands"},
    {"version", cmd_version, "show firmware version"},
    {"info", cmd_info, "device passport"},
    {"uptime", cmd_uptime, "time since reset"},
    {"pi_start", cmd_pi_start, "calculate pi: pi_start [terms]"},
    {"pi", cmd_pi, "pi calculation result or 'busy'"},
    {"profiling", cmd_profiling, "superloop iteration: average and maximum"},
    {"profiling_reset", cmd_profiling_reset, "reset profiling"},
    {"led_enable", cmd_led_enable, "turn LED on"},
    {"led_disable", cmd_led_disable, "turn LED off"},
    {"led_blink", cmd_led_blink, "start LED blinkin"},
    {"led_period", cmd_led_period, "set blink period in ms: led_period <ms>"},
    {"button", cmd_button, "show button state and press count"},
    {"get_voltage", cmd_get_voltage, "voltage on GP26, ADC input 0"},
    {"get_temp", cmd_get_temp, "chip temperature, degrees Celsius"},
    {"tm_start", cmd_tm_start,
     "start telemetry with optional period in ms (default 100)"},
    {"tm_stop", cmd_tm_stop, "stop telemetry"},
    {"charge_on", cmd_charge_on, "turn on RC circuit charge (GP22 high)"},
    {"charge_off", cmd_charge_off, "turn off RC circuit charge (GP22 low)"},
    {"timer_blink_start", cmd_timer_blink_start, "blink led by timer: timer_blink_start <period_ms>"},
    {"timer_blink_stop", cmd_timer_blink_stop, "stop timer blink, number of toggles"},
 	{"cap_start", cmd_cap_start,"start capture: cap_start <N> <period_us>"},
    {"cap_dump", cmd_cap_dump, "dump captured samples"},
    {"dma_cap", cmd_dma_cap,"capture block via DMA: dma_cap <N>"},
    { "stream_start", cmd_stream_start, "start DMA stream: stream_start [8|16]" },
    { "stream_stop", cmd_stream_stop, "stop DMA stream and show stats" },
    { "generate_tone", cmd_generate_tone, "generate tone on GP20: generate_tone <Hz>" },
	{ "hang", cmd_hang, "hang the superloop forever to test watchdog" },
    { "wdt_enable", cmd_wdt_enable, "enable watchdog timer: wdt_enable <timeout_ms>" },
    { "reset_reason", cmd_reset_reason, "show last reset reason and restart count" }
};

const uint32_t api_command_count = sizeof(api_commands) / sizeof(api_commands[0]);
