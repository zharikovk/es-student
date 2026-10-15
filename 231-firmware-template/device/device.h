// device.h
// защита от повторного включения
#pragma once

#include <stdio.h>
#include <stdint.h>
#include <stddef.h>
#define DEVICE_NAME "es-led-module"// имя устройства, версия прошивки
#define FIRMWARE_VERSION "1.1.0"

#define DEVICE_PROJECT "231-firmware-template"
#define DEVICE_REPO "https://github.com/zharikovk/es-student"

#ifndef DEVICE_BOARD
#define DEVICE_BOARD "unknown"
#endif

void device_get_info(char *buffer, size_t max_len);
