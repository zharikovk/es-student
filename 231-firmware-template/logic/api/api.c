// api.c
#include "api.h"
#include "api-commands.h"
#include <stdio.h>
#include <string.h>

void api_init(void) 
{
    // Пока ничего 
}

void api_handle(const command_t *command) 
{
	if (command == 0) return;
    if (command->truncated) 
    {
        printf("error: command longer than 63 characters or 4 arguments\n");
        return;
    }

    if (command->name == 0) 
    {
        return;
    }
	if (strcmp(command->name, "help") == 0) 
	{
        printf("help\n");
        for (uint32_t i = 0; i < api_command_count; i++) 
        {
            printf("  %-15s - %s\n", api_commands[i].name, api_commands[i].help_text);
        }
        return;
    }

    for (uint32_t i = 0; i < api_command_count; i++) 
    {
        if (strcmp(command->name, api_commands[i].name) == 0) 
        {
            api_commands[i].handler(command);
            return;
        }
    }
    printf("error: unknown command '%s', try help\n", command->name);
}
