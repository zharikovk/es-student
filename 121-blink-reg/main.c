#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include "hardware/regs/addressmap.h" //базовые типы и функцию задержки `sleep_ms`
#include "hardware/regs/sio.h" //функции работы с выводами микроконтроллера



const uint LED_PIN = 25; //подключён к выводу `25`

int main() //inlet)
{
    // весь дальнейший код пишем здесь
    gpio_init(LED_PIN);//включает вывод
    gpio_set_dir(LED_PIN,
                 GPIO_OUT); //`gpio_set_dir` задаёт направление: `GPIO_OUT`
                            //означает, что микроконтроллер будет выдавать
                            //напряжение, а не считывать его

    volatile uint32_t *gpio_out_set =
    (uint32_t *)(SIO_BASE + SIO_GPIO_OUT_SET_OFFSET);
volatile uint32_t *gpio_out_clr =
    (uint32_t *)(SIO_BASE + SIO_GPIO_OUT_CLR_OFFSET);
const uint32_t led_mask = 1u << LED_PIN;
 
    while (1)
    {
        // код мигания пишем здесь
        *gpio_out_set=led_mask;//вкл
        sleep_ms(250);//ждем в мс
        *gpio_out_clr=led_mask;//выкл
        sleep_ms(1000);//ждем в мс
    }
}
