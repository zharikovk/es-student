// platform.c
#include "platform.h"          
#include "pico/stdlib.h"    
#include "hardware/adc.h"
//#include <stdio.h>

#define LED_PIN 25
#define BUTTON_PIN 15
#define LINE_PIN 22 // вывод, который через резистор 1 кОм ведёт линию RC-цепи
#define ADC_PIN 26
#define ADC_CHANNEL 0
// опорное напряжение АЦП на плате Pico и число уровней 12-разрядного кода
#define ADC_VREF_V 3.3f
#define ADC_LEVELS 4096.0f



void platform_led_set(bool on) 
{
    gpio_put(LED_PIN, on);
}

bool platform_button_read(void) 
{
    return (gpio_get(BUTTON_PIN) == false);
}
//////////////////////////////////////331
void platform_line_set(platform_line_mode_t mode, bool high)
{
    if (mode == PLATFORM_LINE_PUSH_PULL)
    {
        // драйвер включён и сам задаёт уровень
        gpio_disable_pulls(LINE_PIN);
        gpio_put(LINE_PIN, high);
        gpio_set_dir(LINE_PIN, GPIO_OUT);
        return;
    }

    // открытый сток: в регистре выхода ноль, переключается только режим
    gpio_pull_up(LINE_PIN);
    gpio_put(LINE_PIN, 0);
    gpio_set_dir(LINE_PIN, high ? GPIO_IN : GPIO_OUT);
}

float platform_voltage_read(void)
{
    adc_select_input(ADC_CHANNEL);
    return adc_read() * ADC_VREF_V / ADC_LEVELS;
}


void platform_init(void) 
{
    stdio_init_all();
    //светодиод
    gpio_init(LED_PIN);
    gpio_set_dir(LED_PIN, GPIO_OUT);
    gpio_put(LED_PIN, false);
    // кнопка 
    gpio_init(BUTTON_PIN);
    gpio_set_dir(BUTTON_PIN, GPIO_IN);
    gpio_pull_up(BUTTON_PIN);
	//line uart
	adc_init();// запуск АЦП:
    adc_gpio_init(ADC_PIN);
	gpio_init(LINE_PIN);
    platform_line_set(PLATFORM_LINE_PUSH_PULL, false);
    
//    sleep_ms(5000); 
//    printf("DEBUG INIT: Voltage after force discharge = %.3f V\n", platform_voltage_read());
    
    //  подаём ВЫСОКИЙ уровень (3.3В) на линию
//    platform_line_set(PLATFORM_LINE_PUSH_PULL, true);
//    sleep_ms(100); //  конденсатор зарядился
//    float v_high = platform_voltage_read();
//    printf("DEBUG INIT: Voltage AFTER FORCE HIGH = %.3f V (ОЖИДАЕТСЯ ~3.300)\n", v_high);

    // подаём НИЗКИЙ уровень (0В) на линию
//    platform_line_set(PLATFORM_LINE_PUSH_PULL, false);
//    sleep_ms(100); // конденсатор разрядился
 //   float v_low = platform_voltage_read();
//    printf("DEBUG INIT: Voltage AFTER FORCE LOW = %.3f V (ОЖИДАЕТСЯ ~0.000)\n", v_low);

}
