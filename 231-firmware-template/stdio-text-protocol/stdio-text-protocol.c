// stdio-text-protocol.c

#include "stdio-text-protocol.h"
#include <stdio.h>
#include "pico/stdlib.h"
#include <string.h>

static command_t current_command;
static char line_buffer[STDIO_TEXT_PROTOCOL_LINE_SIZE];
static uint32_t line_len = 0;

void stdio_text_protocol_init(void) {
    line_len = 0;
    line_buffer[0] = '\0';
    current_command.name = NULL;
    current_command.argc = 0;
    current_command.truncated = false;
}

// Вспомогательная функция: разбирает строку на имя и аргументы
static void parse_command(char *str) {
    current_command.name = NULL;
    current_command.argc = 0;
    current_command.truncated = false;

    // 1. Пропускаем начальные пробелы
    while (*str == ' ') {
        str++;
    }

    // 2. Если строка пустая, команды нет
    if (*str == '\0') {
        return;
    }

    // 3. Первое слово - это имя команды
    current_command.name = str;

    // 4. Ищем пробелы, чтобы разделить аргументы
    while (*str != '\0') {
        if (*str == ' ') {
            *str = '\0'; // Заменяем пробел на конец строки, разделяя слова
            str++;       // Переходим к следующему символу
            
            // Пропускаем лишние пробелы между аргументами
            while (*str == ' ') {
                str++;
            }

            // Если еще есть место для аргументов, сохраняем указатель
            if (current_command.argc < STDIO_TEXT_PROTOCOL_MAX_ARGS) {
                current_command.argv[current_command.argc] = str;
                current_command.argc++;
            } else {
                // Аргументов слишком много!
                current_command.truncated = true;
            }
        } else {
            str++;
        }
    }
}

const command_t *stdio_text_protocol_handle(void) {
    // Читаем один символ, не ожидая его (0 микросекунд таймаута)
    // Если ничего не нажато, вернется PICO_ERROR_TIMEOUT
    int c = getchar_timeout_us(0);

    if (c == PICO_ERROR_TIMEOUT) {
        return 0; // Нечего обрабатывать, выходим сразу, не блокируя процессор
    }

    // Обработка нажатия Enter (конец строки)
    if (c == '\r' || c == '\n') {
        putchar('\r');
        putchar('\n'); // Красивый перенос строки в терминале

        if (line_len > 0) {
            line_buffer[line_len] = '\0'; // Завершаем строку нуль-терминатором
            
            // Если строка влезла в буфер целиком, парсим её
            if (!current_command.truncated) {
                parse_command(line_buffer);
            }
            
            line_len = 0; // Сбрасываем счетчик для следующей команды
            return &current_command; // Возвращаем готовую команду!
        }
        
        // Если нажали Enter на пустой строке, просто возвращаем NULL
        return 0;
    }

    // Обработка Backspace (стирание символа)
    // Коды backspace: 8 (\b) или 127 (DEL)
    if (c == '\b' || c == 127) {
        if (line_len > 0) {
            line_len--;
            // Отправляем в терминал последовательность для стирания символа:
            // сдвинуть курсор назад, напечатать пробел, сдвинуть курсор назад
            printf("\b \b");
        }
        return 0;
    }

    // Обработка обычного символа
    if (line_len < STDIO_TEXT_PROTOCOL_LINE_SIZE - 1) {
        line_buffer[line_len] = (char)c;
        line_len++;
        putchar(c); // Эхо: показываем пользователю, что он нажал
    } else {
        // Буфер переполнен!
        current_command.truncated = true;
        line_len = 0; // Сбрасываем, чтобы начать новую строку чисто
        return &current_command;
    }

    return 0; // Строка еще не закончена
}
