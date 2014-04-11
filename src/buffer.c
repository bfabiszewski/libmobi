//
//  buffer.c
//  mobi
//
//  Created by Bartek on 27.03.14.
//  Copyright (c) 2014 Bartek. All rights reserved.
//

#include <stdio.h>
#include "buffer.h"

#define MAX_BUFFER_SIZE    4096

MOBIBuffer * buffer_init(size_t len) {
    MOBIBuffer *p = NULL;
    p = malloc(sizeof(MOBIBuffer));
	if (p == NULL) {
        printf("Buffer allocation failed\n");
        return NULL;
    }
    p->data = malloc(len);
	if (p->data == NULL) {
		free(p);
        printf("Buffer data allocation failed\n");
		return NULL;
	}
	p->offset = 0;
	p->maxlen = len;
	return p;
}


void buffer_add8(MOBIBuffer *p, uint8_t data) {
    if (p->offset + 1 > p->maxlen) {
        printf("Buffer full\n");
        return;
    }
	p->data[p->offset++] = data;
}

void buffer_add16(MOBIBuffer *p, uint16_t data) {
    if (p->offset + 2 > p->maxlen) {
        printf("Buffer full\n");
        return;
    }
	p->data[p->offset++] = (data & 0xff00) >> 8;
    p->data[p->offset++] = (data & 0xff);
}

void buffer_add32(MOBIBuffer *p, uint32_t data) {
    if (p->offset + 4 > p->maxlen) {
        printf("Buffer full\n");
        return;
    }
	p->data[p->offset++] = (data & 0xff000000) >> 16;
	p->data[p->offset++] = (data & 0xff0000) >> 12;
	p->data[p->offset++] = (data & 0xff00) >> 8;
	p->data[p->offset++] = (data & 0xff);
}

void buffer_addraw(MOBIBuffer *p, char* buf, size_t len) {
    if (p->offset + len > p->maxlen) {
        printf("Buffer full\n");
        return;
    }
    memcpy(p->data + p->offset, buf, len);
    p->offset += len;
}

void buffer_addstring(MOBIBuffer *p, char *str) {
    size_t len;
    len = strlen(str);
    buffer_addraw(p, str, len);
}

void buffer_addzeros(MOBIBuffer *p, size_t count) {
    if (p->offset + count > p->maxlen) {
        printf("Buffer full\n");
        return;
    }
    memset(p->data + p->offset, 0, count);
    p->offset += count;
}

uint8_t buffer_get8(MOBIBuffer *p) {
    if (p->offset + 1 > p->maxlen) {
        printf("End of buffer\n");
        return 0;
    }
    return (uint8_t) p->data[p->offset++];
}

uint16_t buffer_get16(MOBIBuffer *p) {
    if (p->offset + 2 > p->maxlen) {
        printf("End of buffer\n");
        return 0;
    }
    uint16_t val;
    val = (uint8_t) p->data[p->offset] << 8 | (uint8_t) p->data[p->offset + 1];
    p->offset += 2;
    return val;
}

uint32_t buffer_get32(MOBIBuffer *p) {
    if (p->offset + 4 > p->maxlen) {
        printf("End of buffer\n");
        return 0;
    }
    uint32_t val;
    val = (uint8_t) p->data[p->offset] << 24 | (uint8_t) p->data[p->offset + 1] << 16 | (uint8_t) p->data[p->offset + 2] << 8 | (uint8_t) p->data[p->offset + 3];
    p->offset += 4;
    return val;
}

void buffer_getstring(char *str, MOBIBuffer *p, size_t len) {
    if (p->offset + len > p->maxlen) {
        printf("End of buffer\n");
        return;
    }
    strncpy(str, p->data + p->offset, len);
    p->offset += len;
}

void buffer_getraw(void *ptr, MOBIBuffer *p, size_t len) {
    if (p->offset + len > p->maxlen) {
        printf("End of buffer\n");
        return;
    }
    memcpy(ptr, p->data + p->offset, len);
    p->offset += len;
}

void buffer_copy8(uint8_t **val, MOBIBuffer *p) {
    *val = NULL;
    if (p->offset + 1 > p->maxlen) {
        return;
    }
    *val = malloc(sizeof(uint8_t));
    if (*val == NULL) {
        return;
    }
    **val = (uint8_t) p->data[p->offset++];
}

void buffer_copy16(uint16_t **val, MOBIBuffer *p) {
    *val = NULL;
    if (p->offset + 2 > p->maxlen) {
        return;
    }
    *val = malloc(sizeof(uint16_t));
    if (*val == NULL) {
        return;
    }
    **val = (uint8_t) p->data[p->offset] << 8 | (uint8_t) p->data[p->offset + 1];
    p->offset += 2;
}

void buffer_copy32(uint32_t **val, MOBIBuffer *p) {
    *val = NULL;
    if (p->offset + 4 > p->maxlen) {
        return;
    }
    *val = malloc(sizeof(uint32_t));
    if (*val == NULL) {
        return;
    }
    **val = (uint8_t) p->data[p->offset] << 24 | (uint8_t) p->data[p->offset + 1] << 16 | (uint8_t) p->data[p->offset + 2] << 8 | (uint8_t) p->data[p->offset + 3];
    p->offset += 4;
}

int is_littleendian() {
    volatile uint32_t i = 1;
    return (*((uint8_t*)(&i))) == 1;
}

uint32_t endian_swap32(uint32_t x) {
    return
    (x & 0xff) << 24 |
    (x & 0xff00) << 8 |
    (x & 0xff0000) >> 8 |
    (x & 0xff000000) >> 24;
}

void buffer_free(MOBIBuffer *p) {
	if (p == NULL) return;
    
	if (p->data != NULL) {
		free(p->data);
	}
	free(p);
}
