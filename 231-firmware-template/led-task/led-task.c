#include "led-task.h"
#include "pico/stdlib.h"
#include "systime/systime.h"

#define PICO_DEFAULT_LED_PIN 25
// Состояние задачи: внутреннее связывание, только для этого файла
static led_state_t led_state;
static uint64_t half_period_us=500000;
static uint64_t last_toggle_us;
static uint32_t current_period_ms = 1000;
// Вспомогательная функция: из других файлов её не вызвать
static void led_set(bool on)
{
    gpio_put(PICO_DEFAULT_LED_PIN, on);
}

bool led_task_set_period_ms(uint32_t period_ms)
{
    // С нулевым полупериодом светодиод переключался бы
    // на каждой итерации: такой период не принимается
    if (period_ms == 0) 
    {
        return false;
    }
	current_period_ms=period_ms;
    half_period_us = (uint64_t)period_ms * 1000 / 2;
    last_toggle_us=systime_us();
    return true;
}
////////////////////////
uint32_t led_task_get_period_ms(void) 
{
    return current_period_ms;
}
////////////////////////////////////////////////////////////
void led_task_init(void)
{
    // настраиваем вывод светодиода, период 1000 мс, состояние «мигает»
    gpio_init(PICO_DEFAULT_LED_PIN);
    gpio_set_dir(PICO_DEFAULT_LED_PIN, GPIO_OUT);
    gpio_put(PICO_DEFAULT_LED_PIN, 0); // Изначально выключен
    led_state = LED_STATE_OFF;
    last_toggle_us = systime_us();
    led_task_set_period_ms(1000); 
}

void led_task_set_state(led_state_t state)
{
    // запоминаем состояние и выполняем действие при входе:
    // «выключен» — гасим, «горит» — зажигаем, «мигает» — первое переключение сразу
    led_state = state;

    // Действие при входе: выполняется один раз, в момент перехода
    switch (led_state) {
    case LED_STATE_OFF:
        gpio_put(PICO_DEFAULT_LED_PIN, false);
        break;
    case LED_STATE_ON:
        gpio_put(PICO_DEFAULT_LED_PIN, true);
        break;
    case LED_STATE_BLINK:
        // Первое переключение — на ближайшей итерации
        last_toggle_us = 0;
        break;
    }
}

void led_task_handle(void)
{
    switch (led_state) 
    {
    case LED_STATE_OFF:
    case LED_STATE_ON:
        // Светодиод установлен при входе в состояние, делать нечего
        break;
    case LED_STATE_BLINK:
        if (systime_period_elapsed(&last_toggle_us, half_period_us)) {
            gpio_put(PICO_DEFAULT_LED_PIN, !gpio_get(PICO_DEFAULT_LED_PIN));
        }
        break;
    }
}
    
    // «выключен» и «горит» — ничего не делаем, светодиод установлен при входе
    // «мигает» — каждые полпериода переключаем светодиод
    

/////на кнопку
void led_task_next_state(void) 
{
    switch (led_state) {
        case LED_STATE_OFF:
            led_task_set_state(LED_STATE_ON);
            break;
        case LED_STATE_ON:
            led_task_set_state(LED_STATE_BLINK);
            break;
        case LED_STATE_BLINK:
            led_task_set_state(LED_STATE_OFF);
            break;
    }
}
