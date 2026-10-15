// device.h
// защита от повторного включения
#pragma once

#include <stdint.h>
#include "pico/unique_id.h" 

#ifndef DEVICE_PROJECT
#define DEVICE_PROJECT "231-firmware-template"
#endif
#ifndef DEVICE_REPO
#define DEVICE_REPO "https://github.com/zharikovk/es-student"
#endif
#ifndef DEVICE_BOARD
#define DEVICE_BOARD "pico"
#endif

typedef struct {
    const char *project;
    const char *repo;
    const char *board;
    char serial[PICO_UNIQUE_BOARD_ID_SIZE_BYTES * 2 + 1];
    uint32_t chip_manufacturer;
    uint32_t chip_part;
    uint32_t chip_revision;
    const char *pico_sdk_version;
} device_info_t;

void device_get_info(device_info_t *info);
