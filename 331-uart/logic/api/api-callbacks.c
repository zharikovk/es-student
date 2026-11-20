// api-callbacks.c
#include "api.h"
#include "api-commands.h"
#include "device/device.h" // модуль железа
#include "firmware.h"
#include "systime/systime.h"
#include "profiling/profiling.h"
#include "platform/platform.h"
#include "stdio-text-protocol/stdio-text-protocol.h"
#include "led-task.h"
#include "button-task.h"
#include "pi-task.h"
#include "edge-task.h"
#include "uart-task.h"
#include <ctype.h>
#include <errno.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define ECHO_MAX_LEN 32
#define BLOCK_SIZE 64
static uint8_t expected_block[BLOCK_SIZE];

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
//////////////////////////////////////331


void print_edge(const char *mode, bool ok, uint32_t rise_us, uint32_t fall_us) 
{// print_edge(): ok == false — edge <pp|od>: timeout;
// иначе edge <pp|od>: rise <мс> ms, fall <мс> ms
    if (!ok) 
    {
        printf("edge %s: timeout\r\n", mode);
    } else 
    {
        //printf("edge %s: rise %lu ms, fall %lu ms\r\n", mode, rise_ms, fall_ms);
        //printf("edge %s: rise %.1f ms, fall %.1f ms\r\n", mode, rise_us / 1000.0f, fall_us / 1000.0f);
     printf("edge %s: rise %.1f ms, fall %.1f ms\r\n", mode, rise_us / 1000.0f, fall_us / 1000.0f);
    }
}
static void edge_measurement_done(platform_line_mode_t mode, bool ok, uint32_t rise_us, uint32_t fall_us) 
{
    const char *mode_str = (mode == PLATFORM_LINE_PUSH_PULL) ? "pp" : "od";
    
    if (!ok) 
    {
        printf("edge %s: timeout\n", mode_str);
    } else 
    {
        printf("edge %s: rise %.1f ms, fall %.1f ms\n", mode_str, rise_us / 1000.0f, fall_us / 1000.0f);
    }
}
void cmd_edge(const command_t *command)
// cmd_edge(): аргумент pp или od выбирает режим, иначе error: usage edge pp|od
// задача не приняла замер — error: edge is busy
{
//printf("DEBUG: argc=%d, argv[0]='%s'\r\n", command->argc, command->argv[0]);
  // printf("DEBUG 1: Команда edge получена!\r\n");    

	if (command->argc != 1) 
	{
        printf("error: usage edge pp|od\r\n");
        return;
    }
    const char *mode_str = command->argv[0];
    platform_line_mode_t mode;
	if (strcmp(mode_str, "pp") == 0) 
	{
        mode = PLATFORM_LINE_PUSH_PULL;
    } else if (strcmp(mode_str, "od") == 0) 
    {
        mode = PLATFORM_LINE_OPEN_DRAIN;
    } else 
    {
        printf("error: usage edge pp|od\r\n");
        return;
    }
	if (!edge_task_start(mode, edge_measurement_done)) 
	{
        printf("error: edge is busy\r\n");
    }
 /*       else 
    {
        printf("DEBUG 2: Задача edge успешно запущена!\r\n");
    } */
}
/////////////////////////////////332

// print_bytes(): байты подряд, непечатные (isprint() == 0) — точкой
static void print_bytes(const uint8_t *data, size_t size) 
{
    for (size_t i = 0; i < size; i++) 
    {
        if (isprint(data[i])) 
        {
            putchar(data[i]);
        } else 
        {
            putchar('.');
        }
    }
}
// print_errors(): err: none или err: и через пробел FE, PE, BE, OE по битам признаков
static void print_errors(uint32_t errors) 
{
    if (errors == 0) 
    {
        printf("err: none\n");
    } else 
    {
        printf("err:");
        if (errors & PLATFORM_UART_FE) printf(" FE");
        if (errors & PLATFORM_UART_PE) printf(" PE");
        if (errors & PLATFORM_UART_BE) printf(" BE");
        if (errors & PLATFORM_UART_OE) printf(" OE");
        printf("\n");
    }
}
// cmd_loop(): аргумент internal или pins, иначе error: usage loop internal|pins
static void cmd_loop(const command_t *command) 
{
    // Должен быть ровно 1 аргумент
    if (command->argc != 1) 
    {
        printf("error: usage loop internal|pins\n");
        return;
    }

    if (strcmp(command->argv[0], "internal") == 0) 
    {
        platform_uart_loop_set(true);
        printf("loop: internal\n");
    } else if (strcmp(command->argv[0], "pins") == 0) 
    {
        platform_uart_loop_set(false);
        printf("loop: pins\n");
    } else 
    {
        printf("error: usage loop internal|pins\n");
    }
}
// platform_uart_loop_set(), ответ loop: <аргумент>

// print_echo(): rx: <принятое> <признаки>, например rx: ping err: none
static void print_echo(const uint8_t *data, size_t size, uint32_t errors) 
{
    printf("rx: ");
    print_bytes(data, size);
    printf(" ");
    print_errors(errors);
}
// cmd_echo(): аргумент не один — error: usage echo <text>
static void cmd_echo(const command_t *command) 
{
    // Должен быть ровно 1 аргумент (сам текст)
    if (command->argc != 1) 
    {
        printf("error: usage echo <text>\n");
        return;
    }

    const char *text = command->argv[0];
    size_t len = strlen(text);

    // Проверка на превышение длины
    if (len > ECHO_MAX_LEN) 
    {
        printf("error: text longer than 32\n");
        return;
    }

    // Запуск задачи обмена (UART0 передаёт и UART0 принимает)
    if (!uart_task_start(PLATFORM_UART0, PLATFORM_UART0, (const uint8_t *)text, len, print_echo)) {
        printf("error: uart is busy\n");
    }
}
// строка длиннее 32 символов — error: text longer than 32
// задача не приняла обмен — error: uart is busy
// иначе обмен UART0 → UART0 с функцией print_echo
/////////////////////////////////333
// строка команды text: ровно 32 символа, столько же мест в очереди приёма
static const char text_line[] = "0123456789ABCDEFGHIJKLMNOPQRSTUV";
static void print_link_back(const uint8_t *data, size_t size, uint32_t errors) 
{
    printf("1->0: ");
    print_bytes(data, size);
    printf(" ");
    print_errors(errors);
}
// print_link_forth(): 0->1: <принятое> <признаки>, затем обмен UART1 → UART0
static void print_link_forth(const uint8_t *data, size_t size, uint32_t errors) 
{
    printf("0->1: ");
    print_bytes(data, size);
    printf(" ");
    print_errors(errors);
    
    // 
    //const uint8_t text[] = "link";
    //uart_task_start(PLATFORM_UART1, PLATFORM_UART0, text, 4, print_link_back);
    uart_task_start(PLATFORM_UART1, PLATFORM_UART0, data, size, print_link_back);

}
// с принятыми байтами и функцией print_link_back
// print_link_back(): 1->0: <принятое> <признаки>
// cmd_link(): проверки аргумента как у echo, обмен UART0 → UART1 с print_link_forth
static void cmd_link(const command_t *command) 
{
    if (command->argc != 1) 
    {
        printf("error: usage link <text>\n");
        return;
    }
    
    const char *text = command->argv[0];
    size_t len = strlen(text);
    
    if (len > 32) 
    {
        printf("error: text longer than 32\n");
        return;
    }

    if (!uart_task_start(PLATFORM_UART0, PLATFORM_UART1, (const uint8_t *)text, len, print_link_forth)) 
    {
        printf("error: uart is busy\n");
    }
}
// cmd_baud_shift(): аргумент не одно целое число — error: usage baud_shift <percent>
// процент вне −50..50 — error: percent must be -50..50
// скорость UART1 = 115200 * (100 + процент) / 100,
// ответ baud_shift: uart1 <скорость из platform_uart_baud_set()> baud
static void cmd_baud_shift(const command_t *command) 
{
    if (command->argc != 1) 
    {
        printf("error: usage baud_shift <percent>\n");
        return;
    }
    char *endptr;
    long percent = strtol(command->argv[0], &endptr, 10); 
    // процент вне −50..50 — error: percent must be -50..50
    if (*endptr != '\0' || percent < -50 || percent > 50) 
    {
        printf("error: percent must be -50..50\n");
        return;
    }
    // скорость UART1 = 115200 * (100 + процент) / 100
    uint32_t new_baud = 115200 * (100 + (uint32_t)percent) / 100; 
    // ответ baud_shift: uart1 <скорость из platform_uart_baud_set()> baud
    uint32_t actual_baud = platform_uart_baud_set(PLATFORM_UART1, new_baud);
    printf("baud_shift: uart1 %lu baud\n", (unsigned long)actual_baud);
}
// print_text(): text: <число байтов> chars <принятое> <признаки>
static void print_text(const uint8_t *data, size_t size, uint32_t errors) 
{
    printf("text: %zu chars ", size);
    print_bytes(data, size);
    printf(" ");
    print_errors(errors);
}
// cmd_text(): обмен UART1 → UART0 со строкой text_line и функцией print_text
static void cmd_text(const command_t *command) 
{
    if (command->argc != 0) 
    {
        printf("error: usage text\n");
        return;
    }
    
    // text_line имеет длину 32. sizeof даст 33 из-за '\0', поэтому передаём ровно 32
    if (!uart_task_start(PLATFORM_UART1, PLATFORM_UART0, (const uint8_t *)text_line, 32, print_text)) 
    {
        printf("error: uart is busy\n");
    }
}
// cmd_parity(): два аргумента из none, even, odd, иначе
// error: usage parity none|even|odd none|even|odd
// первый — UART0, приёмник text, второй — UART1, передатчик text,
// ответ parity: uart0 <первый>, uart1 <второй>
static void cmd_parity(const command_t *command) 
{
    if (command->argc != 2) 
    {
        printf("error: usage parity none|even|odd none|even|odd\n");
        return;
    }

    const char *arg0 = command->argv[0]; // для UART0 (приёмник)
    const char *arg1 = command->argv[1]; // для UART1 (передатчик)

    
	platform_parity_t p0 = PLATFORM_PARITY_NONE; 
    if (strcmp(arg0, "even") == 0) p0 = PLATFORM_PARITY_EVEN;
    else if (strcmp(arg0, "odd") == 0) p0 = PLATFORM_PARITY_ODD;

    platform_parity_t p1 = PLATFORM_PARITY_NONE;
    if (strcmp(arg1, "even") == 0) p1 = PLATFORM_PARITY_EVEN;
    else if (strcmp(arg1, "odd") == 0) p1 = PLATFORM_PARITY_ODD;
    platform_uart_parity_set(PLATFORM_UART0, p0);
    platform_uart_parity_set(PLATFORM_UART1, p1);
	printf("parity: uart0 %s, uart1 %s\n", arg0, arg1);
}
//////////////////////////////////////334
static const char* current_block_mode = "poll";

static void print_block(const uint8_t *data, size_t size, uint32_t errors) 
{
    platform_uart_rx_irq_set(false);
    bool match = (size > 0) && (memcmp(data, expected_block, size) == 0); 
    printf("block %s: %zu/64 %s, ", current_block_mode, size, match ? "match" : "mismatch");
    print_errors(errors);
}


static void cmd_block(const command_t *command) 
{
    if (command->argc != 1) 
    {
        printf("error: usage block irq|poll\n");
        return;
    }

    const char *mode_str = command->argv[0];
    bool use_irq = false;

    if (strcmp(mode_str, "irq") == 0) 
    {
        use_irq = true;
    } else if (strcmp(mode_str, "poll") == 0) 
    {
        use_irq = false;
    } else 
    {
        printf("error: usage block irq|poll\n");
        return;
    }
    for (size_t i = 0; i < BLOCK_SIZE; i++) 
    {
        expected_block[i] = (uint8_t)i;
    }
    current_block_mode = mode_str;
    platform_uart_rx_irq_set(use_irq);
    if (!uart_task_start(PLATFORM_UART1, PLATFORM_UART0, expected_block, BLOCK_SIZE, print_block)) 
    {
        platform_uart_rx_irq_set(false);
        printf("error: uart is busy\n");
    }
}

/////////////////////////////////
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
    {"edge", cmd_edge , "rise and fall of the RC line: edge pp|od"},
    {"loop", cmd_loop, "UART0 loopback: loop internal|pins" },
	{"echo", cmd_echo, "text through UART0 and back: echo <text>" },
	{"link", cmd_link , "text from UART0 to UART1 and back: link <text>"},
	{"baud_shift", cmd_baud_shift, "shift UART1 baud rate: baud_shift <percent>" },
	{"text", cmd_text, "32 characters from UART1 to UART0" },
	{"parity", cmd_parity, "parity of UART0 and UART1: parity <rx> <tx>" },
	{ "block", cmd_block, "64 bytes from UART1 to UART0: block irq|poll"}
};

const uint32_t api_command_count = sizeof(api_commands) / sizeof(api_commands[0]);
