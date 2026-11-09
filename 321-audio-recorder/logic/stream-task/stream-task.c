//stream-task.c
#include "stream-task.h"
#include "platform/platform.h"
#include <stddef.h>

static stream_state_t state = STREAM_STATE_IDLE;
static stream_task_callback_t user_callback = NULL;
static uint8_t current_bits = 0;
static uint16_t frame_number = 0;
static uint32_t total_frames = 0;
static uint32_t total_samples = 0;
// stream_task_init(): запомнить функцию для кадров, состояние IDLE
void stream_task_init(stream_task_callback_t on_frame)
{
    user_callback = on_frame;
    state = STREAM_STATE_IDLE;
    current_bits = 0;
    frame_number = 0;
    total_frames = 0;
    total_samples = 0;
}
// stream_task_start(): поток уже идёт или platform его не начал — false;
// запомнить разрядность, номер кадра и итог — с нуля, состояние RUN
bool stream_task_start(uint8_t bits)
{
    // если поток уже идёт или разрядность не 8/16 — отказываем
    if (state == STREAM_STATE_RUN || (bits != 8 && bits != 16)) 
    {
        return false;
    }
    // пытаемся запустить на уровне platform
    if (!platform_stream_start(bits)) 
    {
        return false;
    }
    // успешный запуск: инициализируем счётчики и состояние
    state = STREAM_STATE_RUN;
    current_bits = bits;
    frame_number = 0;
    total_frames = 0;
    total_samples = 0;
    return true;
}
// stream_task_stop(): остановить поток в platform, состояние IDLE
void stream_task_stop(void)
{
    if (state == STREAM_STATE_RUN) 
    {
        platform_stream_stop();
        state = STREAM_STATE_IDLE;
    }
}
// stream_task_handle(): в состоянии RUN, если platform отдал буфер:
void stream_task_handle(void)
{
    if (state != STREAM_STATE_RUN) 
    {
        return;
    }

    uint32_t samples_count = 0;
    const uint8_t *buffer = platform_stream_next(&samples_count);
    if (buffer != NULL) 
    {
//     заголовок — синхрослово STREAM_SYNC_WORD, номер, число отсчётов
        stream_header_t header;
        header.sync_word = STREAM_SYNC_WORD; // 0x5aa5 (в little-endian памяти RP2040 это байты a5 5a, как в ТЗ)
        header.number = frame_number;
        header.count = (uint16_t)samples_count;
//     байтов — отсчётов × 2 при 16 битах, отсчётов при 8
        uint32_t bytes = (current_bits == 16) ? (samples_count * 2) : samples_count;
//     отдать заголовок и буфер функции, буфер вернуть platform
        if (user_callback != NULL) 
        {
            user_callback(&header, buffer, bytes);
        }
        platform_stream_release();
//     номер, число кадров и отсчётов — вперёд
        frame_number++;
        total_frames++;
        total_samples += samples_count;
    }
}

stream_state_t stream_task_get_state(void)
{
    return state;
}

uint8_t stream_task_get_bits(void)
{
    return current_bits;
}

uint32_t stream_task_get_frames(void)
{
    return total_frames;
}

uint32_t stream_task_get_samples(void)
{
    return total_samples;
}

uint32_t stream_task_get_overruns(void)
{
    return platform_stream_overruns();
}
