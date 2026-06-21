/**
 * @file ring_buffer_lib.h
 * @brief A collection of multi-core safe functions for managing ring buffers
 * @note This library depends on the Raspberry Pi Pico SDK for
 * managing critical sections
 *
 * MIT License
 * Copyright (c) 2022 rppicomidi
 */
#ifndef RING_BUFFER_LIB_H
#define RING_BUFFER_LIB_H
#include <stdint.h>
#include "pico/stdlib.h"
#include "ring_buffer_lib_config.h"

#ifndef RING_BUFFER_SIZE_TYPE
#define RING_BUFFER_SIZE_TYPE uint8_t
#endif
#ifndef RING_BUFFER_MULTICORE_SUPPORT
#define RING_BUFFER_MULTICORE_SUPPORT 0
#endif
#if RING_BUFFER_MULTICORE_SUPPORT
#include "pico/critical_section.h"
#else
#include "pico/sync.h"
#endif
#ifdef __cplusplus
extern "C" {
#endif
#ifndef RING_BUFFER_ENTER_CRITICAL
#define RING_BUFFER_ENTER_CRITICAL(X) \
    uint32_t X; \
    do { X=save_and_disable_interrupts();} while(0)
#endif
#ifndef RING_BUFFER_EXIT_CRITICAL
#define RING_BUFFER_EXIT_CRITICAL(X) \
    do {restore_interrupts(X); } while(0)
#endif

typedef struct ring_buffer_s
{
    uint8_t *buf;
    RING_BUFFER_SIZE_TYPE bufsize;
    RING_BUFFER_SIZE_TYPE in_idx;
    RING_BUFFER_SIZE_TYPE out_idx;
    RING_BUFFER_SIZE_TYPE num_buffered;
#if RING_BUFFER_MULTICORE_SUPPORT
    critical_section_t crit;
#else
    uint32_t critical_section_data;
#endif
} ring_buffer_t;

#if RING_BUFFER_MULTICORE_SUPPORT
void ring_buffer_init(ring_buffer_t *ring_buf, uint8_t* buf, RING_BUFFER_SIZE_TYPE buf_len, uint lock_num);
#else
void ring_buffer_init(ring_buffer_t *ring_buf, uint8_t* buf, RING_BUFFER_SIZE_TYPE buf_len, uint32_t critical_section_data);
#endif

RING_BUFFER_SIZE_TYPE ring_buffer_push_unsafe(ring_buffer_t *ring_buf, const uint8_t* vals, RING_BUFFER_SIZE_TYPE nvals);
RING_BUFFER_SIZE_TYPE ring_buffer_push(ring_buffer_t *ring_buf, const uint8_t* vals, RING_BUFFER_SIZE_TYPE nvals);
RING_BUFFER_SIZE_TYPE ring_buffer_get_num_bytes_unsafe(ring_buffer_t *ring_buf);
RING_BUFFER_SIZE_TYPE ring_buffer_get_num_bytes(ring_buffer_t *ring_buf);
bool ring_buffer_is_full_unsafe(ring_buffer_t *ring_buf);
bool ring_buffer_is_full(ring_buffer_t *ring_buf);
bool ring_buffer_is_empty_unsafe(ring_buffer_t *ring_buf);
bool ring_buffer_is_empty(ring_buffer_t *ring_buf);
RING_BUFFER_SIZE_TYPE ring_buffer_pop_unsafe(ring_buffer_t *ring_buf, uint8_t* vals, RING_BUFFER_SIZE_TYPE maxvals);
RING_BUFFER_SIZE_TYPE ring_buffer_pop(ring_buffer_t *ring_buf, uint8_t* vals, RING_BUFFER_SIZE_TYPE maxvals);
RING_BUFFER_SIZE_TYPE ring_buffer_peek_unsafe(ring_buffer_t *ring_buf, uint8_t* vals, RING_BUFFER_SIZE_TYPE maxvals);
RING_BUFFER_SIZE_TYPE ring_buffer_peek(ring_buffer_t *ring_buf, uint8_t* vals, RING_BUFFER_SIZE_TYPE maxvals);

#ifdef __cplusplus
}
#endif
#endif /* RING_BUFFER_LIB_H */
