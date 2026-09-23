// device.h
// защита от повторного включения
#pragma once

#include <stdio.h>

#define DEVICE_NAME "es-led-module"// имя устройства, версия прошивки
#define FIRMWARE_VERSION "1.0.0"

#define DEVICE_PROJECT "211-command-usb"
#define DEVICE_REPO "https://github.com/zharikovk/es-student"

#ifndef DEVICE_BOARD
#define DEVICE_BOARD "unknown"
#endif

void device_info(void);

