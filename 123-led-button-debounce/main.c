#include "pico/stdlib.h"// добавляем заголовочный файл функций ввода-вывода
#include "hardware/gpio.h"// добавляем заголовочный файл функций работы с GPIO

const uint LED_PIN = 25; //Светодиод на плате Raspberry Pi Pico подключён к выводу `25`.
const uint BUTTON_PIN = 15; //// объявляем константу вывода светодиода

const uint DEBOUNCE_MS = 20; /// Заведите константу задержки

bool get_button_debounce(uint pin)//Заведите константу задержки и функцию, которая читает вывод устойчиво
{
    bool state = gpio_get(pin);
    sleep_ms(DEBOUNCE_MS);
    return state && gpio_get(pin);
}/// 
int main() //inlet)
{
    // весь дальнейший код пишем здесь
    gpio_init(BUTTON_PIN);
    gpio_set_dir(BUTTON_PIN, GPIO_IN);
    gpio_pull_up(BUTTON_PIN);

    gpio_init(LED_PIN);// инициализируем пин светодиода
    gpio_set_dir(LED_PIN, GPIO_OUT); // Настраиваем пин на ВЫХОД
    bool led = false;
    bool previous = false;
     
    while (1)
	{
	    
	    bool current = get_button_debounce(BUTTON_PIN);//Теперь в суперцикле замените прямое чтение вывода вызовом функции. Больше в программе не меняется ничего:
        if (previous == true && current == false)
        {
            led = !led;
            gpio_put(LED_PIN, led);
        }

        previous = current;
    }
}
