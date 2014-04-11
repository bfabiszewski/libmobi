//
//  debug.c
//  mobi
//
//  Created by Bartek on 02.04.14.
//  Copyright (c) 2014 Bartek. All rights reserved.
//

#include "debug.h"

#include <stdlib.h>

#if MOBI_DEBUG
// debug
void debug_free(void *ptr, char *file, int line){
    printf("%s:%d: free(%p)\n",file, line, ptr);
    (free)(ptr);
}

void *debug_malloc(size_t size, char *file, int line) {
    void *ptr = (malloc)(size);
    printf("%s:%d: malloc(%d)=%p\n", file, line, (int)size, ptr);
    return ptr;
}

void *debug_realloc(void *ptr, size_t size, char *file, int line) {
    printf("%s:%d: realloc(%p", file, line, ptr);
    void *rptr = (realloc)(ptr, size);
    printf(", %d)=%p\n", (int)size, rptr);
    return rptr;
}

void *debug_calloc(size_t num, size_t size, char *file, int line) {
    void *ptr = (calloc)(num, size);
    printf("%s:%d: calloc(%d, %d)=%p\n", file, line, (int)num, (int)size, ptr);
    return ptr;
}
#endif
