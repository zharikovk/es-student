// platform.h

#pragma once

#include <stdbool.h>
#include <stdint.h> 
// запускает стандартный ввод-вывод и настраивает выводы

typedef enum {
    BUTTON_1 = 0,
    BUTTON_2 = 1,
    BUTTON_COUNT // Всего кнопок (равно 2)
} platform_button_t;
void platform_init(void);

void platform_led_set(bool on);

// нажата ли кнопка сейчас, без подавления дребезга
bool platform_button_read(platform_button_t button_state);
