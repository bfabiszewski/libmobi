//
//  debug.h
//  mobi
//
//  Created by Bartek on 02.04.14.
//  Copyright (c) 2014 Bartek. All rights reserved.
//

#ifndef mobi_debug_h
#define mobi_debug_h

#include <stdio.h>

#define MOBI_DEBUG 0
#if MOBI_DEBUG
#define free(x) debug_free(x,__FILE__,__LINE__)
void debug_free(void *ptr, char *file, int line);
#define malloc(x) debug_malloc(x, __FILE__, __LINE__ )
void *debug_malloc(size_t size, char *file, int line);
#define realloc(x, y) debug_realloc(x, y, __FILE__, __LINE__ )
void *debug_realloc(void *ptr, size_t size, char *file, int line);
#define calloc(x, y) debug_calloc(x, y, __FILE__, __LINE__ )
void *debug_calloc(size_t num, size_t size, char *file, int line);
#endif


#endif
