// device.h
// защита от повторного включения
#pragma once

#include <stdint.h>
#include "pico/unique_id.h" 

typedef struct 
{
    const char *project;
    const char *repo;
    const char *board;
    char serial[PICO_UNIQUE_BOARD_ID_SIZE_BYTES * 2 + 1];
    uint32_t chip_manufacturer;
    uint32_t chip_part;
    uint32_t chip_revision;
    const char *pico_sdk_version;
} device_info_t;

void device_get_info(device_info_t *info,
	const char *project,
    const char *repo,
    const char *board);
