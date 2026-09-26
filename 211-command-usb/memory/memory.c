// memory.c

#include <stdio.h>
#include <stdint.h>
#include "pico/stdlib.h"//даёт функцию чтения серийного номера
#include "hardware/regs/addressmap.h"//базовый адрес `SYSINFO_BASE
#include "memory.h"

extern char __flash_binary_start;
extern char __flash_binary_end;
extern char __boot2_start__;
extern char __boot2_end__;
extern char __etext;
extern char __data_start__;
extern char __data_end__;
extern char __bss_start__;
extern char __bss_end__;
extern char __HeapLimit;
extern char __StackBottom;
extern char __StackTop;



static void row(const char *name, uintptr_t start, uintptr_t end)
{
    printf("%-10s 0x%08x 0x%08x %8u\n",
           name, (unsigned)start, (unsigned)end, (unsigned)(end - start));
}



void mem_info(void)
{
	printf("area      start      end      size\n");
    // шапка таблицы: область, начало, конец, размер
	row("flash", (uintptr_t)XIP_BASE, (uintptr_t)XIP_BASE+PICO_FLASH_SIZE_BYTES);
    // flash — XIP_BASE и PICO_FLASH_SIZE_BYTES
    // sram — базовый адрес из SDK, размер из datasheet
    // rom — базовый адрес из SDK, размер из datasheet
	row("sram", (uintptr_t)SRAM_BASE, (uintptr_t)SRAM_END);
	row("rom", (uintptr_t)ROM_BASE, (uintptr_t)ROM_BASE+16*1024);
    // image — от __flash_binary_start до __flash_binary_end
    row("image", (uintptr_t)&__flash_binary_start, (uintptr_t)&__flash_binary_end);
    // free  — от __flash_binary_end до конца флеш-памяти
    row("free", (uintptr_t)&__flash_binary_end, (uintptr_t)XIP_BASE + PICO_FLASH_SIZE_BYTES);
    // boot2 — от __boot2_start__ до __boot2_end__
    row("boot2", (uintptr_t)&__boot2_start__, (uintptr_t)&__boot2_end__);
    // text  — от __boot2_end__ до __etext: код и константы
	row("text", (uintptr_t)&__boot2_end__, (uintptr_t)&__etext);
    // data flash — хранение .data, от __etext, длиной с .data
    uintptr_t data_size = (uintptr_t)&__data_end__ - (uintptr_t)&__data_start__;
    row("data flash", (uintptr_t)&__etext, (uintptr_t)&__etext+data_size);
    // data ram   — работа .data, от __data_start__ до __data_end__
    row("data ram", (uintptr_t)&__data_start__, (uintptr_t)&__data_end__);
    // bss        — от __bss_start__ до __bss_end__
    row("bss", (uintptr_t)&__bss_start__, (uintptr_t)&__bss_end__);
    // heap       — от __bss_end__ до __HeapLimit
    row("heap", (uintptr_t)&__bss_end__, (uintptr_t)&__HeapLimit);
    // stack      — от __StackBottom до __StackTop
    row("stack", (uintptr_t)&__StackBottom, (uintptr_t)&__StackTop);

    // итог: образ во флеш и из чего он сложился
    printf("\n");
    printf("  total\n");

    // итог: свободно во флеш-памяти из всего её объёма
    // итог: занято в ОЗУ — .data и .bss
    // итог: свободно в ОЗУ — под кучу и под стек
    unsigned image_size = (uintptr_t)&__flash_binary_end - (uintptr_t)&__flash_binary_start;
    unsigned boot2_size = (uintptr_t)&__boot2_end__ - (uintptr_t)&__boot2_start__;
    unsigned text_size  = (uintptr_t)&__etext - (uintptr_t)&__boot2_end__;
    unsigned data_sz    = (uintptr_t)&__data_end__ - (uintptr_t)&__data_start__;

    // итог: образ во флеш и из чего он сложился
    printf("  flash image: %8u = boot2 %6u + text %6u + data %6u bytes\n",
           image_size, boot2_size, text_size, data_sz);

    // итог: свободно во флеш-памяти из всего её объёма
    unsigned free_flash = PICO_FLASH_SIZE_BYTES - image_size;
    printf("  free flash:  %8u bytes of %8u bytes\n", free_flash, PICO_FLASH_SIZE_BYTES);

    // итог: занято в ОЗУ — .data и .bss
    unsigned bss_size = (uintptr_t)&__bss_end__ - (uintptr_t)&__bss_start__;
    unsigned used_ram = data_sz + bss_size;
    printf("  ram used: %8u bytes =(.data %8u + .bss %8u)\n", used_ram, data_sz,bss_size);

    // итог: свободно в ОЗУ — под кучу и под стек
    unsigned free_ram = ((uintptr_t)&__HeapLimit - (uintptr_t)&__bss_end__) +
                        ((uintptr_t)&__StackTop - (uintptr_t)&__StackBottom);
    unsigned stack = ((uintptr_t)&__StackTop - (uintptr_t)&__StackBottom);               
    printf("  ram free    %8u bytes for heap and  %8u for stack\n", free_ram, stack);
}
