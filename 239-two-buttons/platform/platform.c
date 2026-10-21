// platform.c
#include "platform.h"          
#include "pico/stdlib.h"    

#define LED_PIN 25
#define BUTTON_1_PIN 15
#define BUTTON_2_PIN 14

void platform_init(void) 
{
    stdio_init_all();
    //светодиод
    gpio_init(LED_PIN);
    gpio_set_dir(LED_PIN, GPIO_OUT);
    gpio_put(LED_PIN, false);
    // кнопка 
    gpio_init(BUTTON_1_PIN);
    gpio_set_dir(BUTTON_1_PIN, GPIO_IN);
    gpio_pull_up(BUTTON_1_PIN);

    gpio_init(BUTTON_2_PIN);
    gpio_set_dir(BUTTON_2_PIN, GPIO_IN);
    gpio_pull_up(BUTTON_2_PIN);
}

void platform_led_set(bool on) 
{
    gpio_put(LED_PIN, on);
}

bool platform_button_read(platform_button_t button_state) 
{
    if (button_state == BUTTON_1) 
    {
        return (gpio_get(BUTTON_1_PIN) == false);
    } else if (button_state == BUTTON_2) 
    {
        return (gpio_get(BUTTON_2_PIN) == false);
    }
    return false;
}
