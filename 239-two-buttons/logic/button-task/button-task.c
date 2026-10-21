#include "button-task.h"
#include "platform/platform.h"
#include "systime/systime.h"
#include <stddef.h>
#define DEBOUNCE_MS 20000
#define LONG_PRESS_US 1000000 // 1 секунда

typedef enum {
    STATE_RELEASED,       // Кнопка отпущена
    STATE_MAYBE_PRESSED,  // Возможно нажата (ждем 20 мс)
    STATE_PRESSED,        // Кнопка нажата
    STATE_MAYBE_RELEASED  // Возможно отпущена (ждем 20 мс)
} button_state_t;

static button_state_t btn_state[BUTTON_COUNT] = {0};
static uint64_t last_change_us[BUTTON_COUNT] = {0};
static uint32_t press_count[BUTTON_COUNT] = {0};
static button_action_callback_t short_press_cb[BUTTON_COUNT] = {0};
static button_action_callback_t long_press_cb[BUTTON_COUNT] = {0};
static bool long_press_triggered[BUTTON_COUNT] = {0};

bool button_task_is_pressed(platform_button_t button_state)
{
   return (btn_state[button_state] == STATE_PRESSED || btn_state[button_state] == STATE_MAYBE_RELEASED);
}
////////////////////////
uint32_t button_task_get_press_count(platform_button_t button_state)
{
    return press_count[button_state];
}
////////////////////////////////////////////////////////////
void button_task_init(platform_button_t button_state, 
					  button_action_callback_t on_short, 
                      button_action_callback_t on_long) 
{
    if (button_state >= BUTTON_COUNT) return;
    short_press_cb[button_state] = on_short;
    long_press_cb[button_state] = on_long;
    btn_state[button_state] = STATE_RELEASED;
    last_change_us[button_state] = systime_us();
    press_count[button_state] = 0;
    long_press_triggered[button_state] = false;
}

void button_task_handle(void)
{
    for (platform_button_t button_state = 0; button_state < BUTTON_COUNT; button_state++)
    {
    	bool press_state = platform_button_read(button_state);// читаем уровень: нажатая кнопка замыкает вывод на землю и даёт ноль
    	uint64_t now_us = systime_us();// читаем часы  
    // Действие при входе: выполняется один раз, в момент перехода
    	switch (btn_state[button_state]) 
    	{
     	// переходы по таблице; при переходе в «нажата»
    	// увеличиваем счётчик и вызываем led_task_next_state()
    	case STATE_RELEASED:
    		if (press_state) 
    		{
             	last_change_us[button_state] = now_us;
             	btn_state[button_state] = STATE_MAYBE_PRESSED;
             	long_press_triggered[button_state] = false; // Сбрасываем флаг при новом нажатии
        	}
    		break;
    	case STATE_MAYBE_PRESSED:
    		if (!press_state) 
    		{
            	btn_state[button_state] = STATE_RELEASED;
        	} else if (now_us - last_change_us[button_state] >= DEBOUNCE_MS) 
        	{
        		btn_state[button_state] = STATE_PRESSED;
        		press_count[button_state]++;// увеличиваем счётчик
        		//if (press_callback[button_state] != 0) 
        		//{
                //    	press_callback[button_state]();
            	//}
        	}
    		break;
    	case STATE_PRESSED:
        	// первое переключение — на ближайшей итерации
        	if (!press_state) 
    		{
             	last_change_us[button_state] = now_us;
             	btn_state[button_state] = STATE_MAYBE_RELEASED;
        	} else {
        	///долгое нажатие
            	if (!long_press_triggered[button_state] && (now_us - last_change_us[button_state] >= LONG_PRESS_US)) 
            	{
                	long_press_triggered[button_state] = true; // ставим флаг, чтобы сработать только ОДИН раз
                        
                    if (long_press_cb[button_state] != NULL) 
                    {
                    	long_press_cb[button_state](); // вызываем обработчик долгого нажатия
                    }
                }       	
        	}
    		break;
    	case STATE_MAYBE_RELEASED:
    		if (press_state) 
    		{
            	btn_state[button_state] = STATE_PRESSED;// дребезг при отпускании
        	} else if (now_us - last_change_us[button_state]>= DEBOUNCE_MS) 
        	{
        		btn_state[button_state] = STATE_RELEASED;
        		if (!long_press_triggered[button_state]) //нет долгого нажатия, вызываем короткое
        		{
                        if (short_press_cb[button_state] != NULL) 
                        {
                            short_press_cb[button_state]();
                        }
                    }
        	}
    		break;
    	}
    }
}



