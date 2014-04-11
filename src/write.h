//
//  write.h
//  mobi
//
//  Created by Bartek on 25.03.14.
//  Copyright (c) 2014 Bartek. All rights reserved.
//

#ifndef mobi_write_h
#define mobi_write_h

#include "mobi.h"

__attribute__((visibility("hidden"))) MOBIBuffer * buffer_init(size_t len);
__attribute__((visibility("hidden"))) void buffer_add8(MOBIBuffer *p, uint8_t data);
__attribute__((visibility("hidden"))) void buffer_add16(MOBIBuffer *p, uint16_t data);
__attribute__((visibility("hidden"))) void buffer_add32(MOBIBuffer *p, uint32_t data);
__attribute__((visibility("hidden"))) void buffer_addstring(MOBIBuffer *p, char *str);
__attribute__((visibility("hidden"))) void buffer_free(MOBIBuffer *p);
#endif
