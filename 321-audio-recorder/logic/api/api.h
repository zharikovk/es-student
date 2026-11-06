// api.h

#pragma once

#include "stdio-text-protocol/stdio-text-protocol.h"

void api_init(void);
void api_handle(const command_t *command);

#include "telemetry-task.h"
// печатает запись телеметрии строкой tm: <время, мкс> <напряжение, В> <температура, °C>
void api_print_telemetry(const telemetry_record_t *record);

