// api.h

#pragma once

#include "stdio-text-protocol/stdio-text-protocol.h"

void api_init(void);
void api_handle(const command_t *command);
