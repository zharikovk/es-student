// device.h
// защита от повторного включения
#pragma once

#include <stdio.h>
#include <stdint.h>
#include "pico/stdlib.h"//даёт функцию чтения серийного номера
#include "hardware/regs/addressmap.h"//базовый адрес `SYSINFO_BASE

//void cmd_mem_info(void);
void mem_info(void);
