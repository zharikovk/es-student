// api-commands.h

#pragma once

#include "stdio-text-protocol/stdio-text-protocol.h"
#include <stdint.h>

// обработчик команды: получает имя и аргументы
typedef void (*api_command_handler_t)(const command_t *command);
// Структура одной строки в таблице команд
// команда прибора: имя, справка для help и обработчик
typedef struct {
    const char *name;
    api_command_handler_t handler;
    const char *help_text;
} api_command_t;


extern const api_command_t api_commands[];
extern const uint32_t api_command_count;
