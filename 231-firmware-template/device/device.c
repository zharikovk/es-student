// device.c

#include <stdio.h>
#include <stddef.h>
#include "pico/version.h"
#include "pico/unique_id.h"//даёт функцию чтения серийного номера
#include "hardware/regs/addressmap.h"//базовый адрес `SYSINFO_BASE
#include "hardware/regs/sysinfo.h"//смещение регистра и имена масок.
#include "pico/version.h"
#include "device.h"


void device_get_info(device_info_t *info)
{// 
	if (!info) return;
    
    info->project = DEVICE_PROJECT;
    info->repo = DEVICE_REPO;
    info->board = DEVICE_BOARD;
    info->pico_sdk_version = PICO_SDK_VERSION_STRING;
    
    pico_get_unique_board_id_string(info->serial, sizeof(info->serial));  
    volatile uint32_t *chip_id = (uint32_t *)(SYSINFO_BASE + SYSINFO_CHIP_ID_OFFSET);
    uint32_t id = *chip_id;
    
    info->chip_manufacturer = (id & SYSINFO_CHIP_ID_MANUFACTURER_BITS) >> SYSINFO_CHIP_ID_MANUFACTURER_LSB;
    info->chip_part = (id & SYSINFO_CHIP_ID_PART_BITS) >> SYSINFO_CHIP_ID_PART_LSB;
    info->chip_revision = (id & SYSINFO_CHIP_ID_REVISION_BITS) >> SYSINFO_CHIP_ID_REVISION_LSB;
}
