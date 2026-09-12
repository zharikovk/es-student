#include <stdio.h>
#include "pico/stdlib.h"// добавляем заголовочный файл функций ввода-вывода

int main() //inlet)
{
    stdio_init_all();
    while (1)
    {
        printf("Hello, world!\n");
        sleep_ms(1000);
    }
}
