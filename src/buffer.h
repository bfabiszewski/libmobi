//
//  buffer.h
//  mobi
//
//  Created by Bartek on 27.03.14.
//  Copyright (c) 2014 Bartek. All rights reserved.
//

#ifndef mobi_buffer_h
#define mobi_buffer_h

#include <stdlib.h>
#include <string.h>

#include "mobi.h"

typedef struct {
    char *data;
    size_t offset;
    size_t maxlen;
} MOBIBuffer;

MOBIBuffer * buffer_init(size_t len);
void buffer_add8(MOBIBuffer *p, uint8_t data);
void buffer_add16(MOBIBuffer *p, uint16_t data);
void buffer_add32(MOBIBuffer *p, uint32_t data);
void buffer_addraw(MOBIBuffer *p, char* buf, size_t len);
void buffer_addstring(MOBIBuffer *p, char *str);
void buffer_addzeros(MOBIBuffer *p, size_t count);
uint8_t buffer_get8(MOBIBuffer *p);
uint16_t buffer_get16(MOBIBuffer *p);
uint32_t buffer_get32(MOBIBuffer *p);
void buffer_copy8(uint8_t **val, MOBIBuffer *p);
void buffer_copy16(uint16_t **val, MOBIBuffer *p);
void buffer_copy32(uint32_t **val, MOBIBuffer *p);
void buffer_getstring(char *str, MOBIBuffer *p, size_t len);
void buffer_getraw(void *ptr, MOBIBuffer *p, size_t len);
void buffer_free(MOBIBuffer *p);
int is_littleendian();
uint32_t endian_swap32(uint32_t x);

#endif
