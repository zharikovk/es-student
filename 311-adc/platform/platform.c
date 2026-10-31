// platform.c
#include "platform.h"          
#include "pico/stdlib.h"    
#include "hardware/adc.h"

#define LED_PIN 25
#define BUTTON_PIN 15
// вход АЦП: вывод GP26, канал 0
#define ADC_PIN 26
#define ADC_CHANNEL 0
// опорное напряжение АЦП на плате Pico и число уровней 12-разрядного кода
#define ADC_VREF_V 3.3f
#define ADC_LEVELS 4096
// встроенный датчик температуры — канал 4
#define ADC_TEMP_CHANNEL 4
// вывод, через резистор заряжающий конденсатор RC-цепи
#define CHARGE_PIN 22

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
    adc_init();// запуск АЦП:
    adc_gpio_init(ADC_PIN);
    // в platform_init(): включить питание датчика
	adc_set_temp_sensor_enabled(true);
	// в platform_init(): GP22 на выход, на выводе ноль
	gpio_init(CHARGE_PIN);
    gpio_set_dir(CHARGE_PIN, GPIO_OUT);//выход
    gpio_put(CHARGE_PIN,false);//ноль на старте
}

void platform_led_set(bool on) 
{
    gpio_put(LED_PIN, on);
}

bool platform_button_read(void) 
{
    return (gpio_get(BUTTON_PIN) == false);
}

float platform_voltage_read(void)
{
    adc_select_input(ADC_CHANNEL);
    return adc_read() * ADC_VREF_V / ADC_LEVELS;
}

float platform_temp_read(void)
{
	adc_select_input(ADC_TEMP_CHANNEL);
	float adc_voltage = adc_read() * ADC_VREF_V / ADC_LEVELS;
	float temperature = 27.0f - ((adc_voltage - 0.706f) / 0.001721f);
	return temperature;
}

// platform_charge_set(on): 3,3 В на GP22 или ноль
void platform_charge_set(bool on) 
{
    gpio_put(CHARGE_PIN, on);
}
