#include "led.h"
#include "hardware/gpio.h"

const uint LED_PIN = 25;

static bool led_state = false;

void led_init(void)//`led_init()` настраивает вывод на выход и гасит светодиод,
{
    gpio_init(LED_PIN);// инициализируем пин светодиода
    gpio_set_dir(LED_PIN, GPIO_OUT); // Настраиваем пин на ВЫХОД// инициализируем пин светодиода и настраиваем его на выход
    led_set(false);
}

void led_set(bool on)//`led_set()` выставляет уровень и запоминает состояние,
{
    led_state = on;
    gpio_put(LED_PIN, led_state);
}

void led_toggle(void)//`led_toggle()` вызывает `led_set()` с обратным значением
{
    led_set(!led_state);//только эита функция отвкчает за светодиод
}

bool led_is_on(void)//`led_is_on()` возвращает запомненное
{
    return led_state;
}
