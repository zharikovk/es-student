// device.c

#include <stdio.h>
#include <stddef.h>
#include "pico/version.h"
#include "pico/unique_id.h"//даёт функцию чтения серийного номера
#include "hardware/regs/addressmap.h"//базовый адрес `SYSINFO_BASE
#include "hardware/regs/sysinfo.h"//смещение регистра и имена масок.
#include "device.h"

void device_get_info(char *buffer, size_t max_len)
{// Прочитать серийный номер платы
    char board_id[PICO_UNIQUE_BOARD_ID_SIZE_BYTES * 2 + 1];
    pico_get_unique_board_id_string(board_id, sizeof(board_id));
    
    volatile uint32_t *chip_id = (uint32_t *)(SYSINFO_BASE + SYSINFO_CHIP_ID_OFFSET);
    uint32_t id = *chip_id;
    
    uint32_t manufacturer = (id & SYSINFO_CHIP_ID_MANUFACTURER_BITS) >> SYSINFO_CHIP_ID_MANUFACTURER_LSB;
    uint32_t part = (id & SYSINFO_CHIP_ID_PART_BITS) >> SYSINFO_CHIP_ID_PART_LSB;
    uint32_t revision = (id & SYSINFO_CHIP_ID_REVISION_BITS) >> SYSINFO_CHIP_ID_REVISION_LSB;

    snprintf(buffer,max_len,
    "project: %s\n" 
    "repo: %s\n" 
    "board: %s\n" 
    "serial: %s\n" 
    "chip: manufacturer 0x%03x, part 0x%04x, revision %u\n" 
    "pico-sdk: %s\n",
     DEVICE_PROJECT,
     DEVICE_REPO,
     DEVICE_BOARD,
     board_id,
     manufacturer, part, revision,
     PICO_SDK_VERSION_STRING);
}
