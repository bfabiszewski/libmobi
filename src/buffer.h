/** @file buffer.h
 *
 * Copyright (c) 2014 Bartek Fabiszewski
 * http://www.fabiszewski.net
 *
 * This file is part of libmobi.
 * Licensed under LGPL, either version 3, or any later.
 * See <http://www.gnu.org/licenses/>
 */

#ifndef libmobi_buffer_h
#define libmobi_buffer_h

#include "config.h"
#include "mobi.h"

/**
 @brief Buffer to read to/write from
 */
typedef struct {
    size_t offset; /**< Current offset in respect to buffer start */
    size_t maxlen; /**< Length of the buffer data */
    unsigned char *data; /**< Pointer to buffer data */
    MOBI_RET error; /**< MOBI_SUCCESS = 0 if operation on buffer is successful, non-zero value on failure */
} MOBIBuffer;

MOBIBuffer * buffer_init(const size_t len);
MOBIBuffer * buffer_init_null(const size_t len);
void buffer_add8(MOBIBuffer *buf, const uint8_t data);
void buffer_add16(MOBIBuffer *buf, const uint16_t data);
void buffer_add32(MOBIBuffer *buf, const uint32_t data);
void buffer_addraw(MOBIBuffer *buf, const unsigned char* data, const size_t len);
void buffer_addstring(MOBIBuffer *buf, const char *str);
void buffer_addzeros(MOBIBuffer *buf, const size_t count);
uint8_t buffer_get8(MOBIBuffer *buf);
uint16_t buffer_get16(MOBIBuffer *buf);
uint32_t buffer_get32(MOBIBuffer *buf);
uint32_t buffer_get_varlen(MOBIBuffer *buf, size_t *len);
uint32_t buffer_get_varlen_dec(MOBIBuffer *buf, size_t *len);
void buffer_dup8(uint8_t **val, MOBIBuffer *buf);
void buffer_dup16(uint16_t **val, MOBIBuffer *buf);
void buffer_dup32(uint32_t **val, MOBIBuffer *buf);
void buffer_getstring(char *str, MOBIBuffer *buf, const size_t len);
void buffer_appendstring(char *str, MOBIBuffer *buf, const size_t len);
void buffer_getraw(void *data, MOBIBuffer *buf, const size_t len);
void buffer_copy8(MOBIBuffer *in, MOBIBuffer *source);
void buffer_copy(MOBIBuffer *dest, MOBIBuffer *source, size_t len);
bool buffer_match_magic(MOBIBuffer *buf, const char *magic);
void buffer_free(MOBIBuffer *buf);
void buffer_free_null(MOBIBuffer *buf);

/**
 @brief Dynamic array of uint32_t values structure
 */
typedef struct {
    uint32_t *data; /**< Array */
    size_t maxsize; /**< Allocated size */
    size_t step; /**< Step by which array will be enlarged if out of memory */
    size_t size; /**< Current size */
} MOBIArray;

MOBIArray * array_init(const size_t len);
MOBI_RET array_insert(MOBIArray *arr, uint32_t value);
void array_sort(MOBIArray *arr, bool unique);
size_t array_size(MOBIArray *arr);
void array_free(MOBIArray *arr);

#endif
