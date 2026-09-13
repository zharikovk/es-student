#ifndef LED_H
#define LED_H

#include "pico/stdlib.h"
//Четыре функции покрывают всё, что программе нужно от светодиода: подготовить вывод, задать состояние, переключить, спросить текущее состояние.
void led_init(void);
void led_set(bool on);
void led_toggle(void);
bool led_is_on(void);

#endif

//или так

//#pragma once
//
//#include "pico/stdlib.h"
//
//void led_init(void);
//void led_set(bool on);
//void led_toggle(void);
//bool led_is_on(void);
