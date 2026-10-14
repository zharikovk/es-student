#include "button-task.h"
#include "pico/stdlib.h"
#include "led-task.h"
#define PICO_DEFAULT_BUTTON_PIN 15
#define DEBOUNCE_MS 20


typedef enum {
    BUTTON_RELEASED,       // Кнопка отпущена
    BUTTON_MAYBE_PRESSED,  // Возможно нажата (ждем 20 мс)
    BUTTON_PRESSED,        // Кнопка нажата
    BUTTON_MAYBE_RELEASED  // Возможно отпущена (ждем 20 мс)
} button_state_t;

static button_state_t button_state = BUTTON_RELEASED;
static uint64_t last_change_us = 0;
static uint32_t press_count = 0;

bool button_task_is_pressed(void)
{
   return (button_state == BUTTON_PRESSED || button_state == BUTTON_MAYBE_RELEASED);
}
////////////////////////
uint32_t button_task_get_press_count(void) 
{
    return press_count;
}
////////////////////////////////////////////////////////////
void button_task_init(void)
{
    // настраиваем вывод кнопки
    gpio_init(PICO_DEFAULT_BUTTON_PIN);
    gpio_set_dir(PICO_DEFAULT_BUTTON_PIN, GPIO_IN);
    gpio_pull_up(PICO_DEFAULT_BUTTON_PIN);
    button_state = BUTTON_RELEASED;
    last_change_us = time_us_64();
    press_count = 0;
}

void button_task_handle(void)
{
    bool press_state = (gpio_get(PICO_DEFAULT_BUTTON_PIN) == false);// читаем уровень: нажатая кнопка замыкает вывод на землю и даёт ноль
    uint64_t now_us = time_us_64();// читаем часы  
    // Действие при входе: выполняется один раз, в момент перехода
    switch (button_state) 
    {
     // переходы по таблице; при переходе в «нажата»
    // увеличиваем счётчик и вызываем led_task_next_state()
    case BUTTON_RELEASED:
    	if (press_state) 
    	{
             last_change_us = now_us;
             button_state = BUTTON_MAYBE_PRESSED;
        }
    	break;
    case BUTTON_MAYBE_PRESSED:
    	if (press_state) 
    	{
            button_state = BUTTON_RELEASED;
        } else if (now_us - last_change_us >= DEBOUNCE_MS) 
        {
        	button_state = BUTTON_PRESSED;
        	press_count++;// увеличиваем счётчик
        	led_task_next_state();//и вызываем led_task_next_state()
        }
    	break;
    case BUTTON_PRESSED:
        // Первое переключение — на ближайшей итерации
        if (press_state) 
    	{
             last_change_us = now_us;
             button_state = BUTTON_MAYBE_RELEASED;
        }
    	break;
    case BUTTON_MAYBE_RELEASED:
    	if (press_state) 
    	{
            button_state = BUTTON_PRESSED;
        } else if (now_us - last_change_us >= DEBOUNCE_MS) 
        {
        	button_state = BUTTON_RELEASED;
        }
    	break;
    }
}



