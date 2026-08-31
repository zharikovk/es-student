#include "pico/stdlib.h" //базовые типы и функцию задержки `sleep_ms`
#include "hardware/gpio.h" //функции работы с выводами микроконтроллера

const uint LED_PIN = 25; //подключён к выводу `25`

int main() //inlet)
{
    // весь дальнейший код пишем здесь
    gpio_init(LED_PIN);//включает вывод
    gpio_set_dir(LED_PIN, GPIO_OUT);//`gpio_set_dir` задаёт направление: `GPIO_OUT` означает, что микроконтроллер будет выдавать напряжение, а не считывать его
    while (1)
    {
        // код мигания пишем здесь
        gpio_put(LED_PIN, 1);//вкл
        sleep_ms(250);//ждем в мс
        gpio_put(LED_PIN, 0);//выкл
        sleep_ms(1000);//ждем в мс
    }
}
