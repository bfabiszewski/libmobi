/** @file debug.c
 *  @brief Debugging functions, enable by running configure --enable-debug
 *
 * Copyright (c) 2014 Bartek Fabiszewski
 * http://www.fabiszewski.net
 *
 * This file is part of libmobi.
 * Licensed under LGPL, either version 3, or any later.
 * See <http://www.gnu.org/licenses/>
 */

#include <stdlib.h>

#include "debug.h"

/**
 @brief Debugging wrapper for free(void *ptr)
 
 @param[in] ptr Pointer
 @param[in] file Calling file
 @param[in] line Calling line
 */
void debug_free(void *ptr, const char *file, const int line) {
    printf("%s:%d: free(%p)\n",file, line, ptr);
    (free)(ptr);
}

/**
 @brief Debugging wrapper for malloc(size_t size)
 
 @param[in] size Size of memory
 @param[in] file Calling file
 @param[in] line Calling line
 @return A pointer to the allocated memory block on success, NULL on failure

 */
void *debug_malloc(const size_t size, const char *file, const int line) {
    void *ptr = (malloc)(size);
    printf("%s:%d: malloc(%d)=%p\n", file, line, (int)size, ptr);
    return ptr;
}

/**
 @brief Debugging wrapper for realloc(void* ptr, size_t size)
 
 @param[in] ptr Pointer
 @param[in] size Size of memory
 @param[in] file Calling file
 @param[in] line Calling line
 @return A pointer to the reallocated memory block on success, NULL on failure
 */
void *debug_realloc(void *ptr, const size_t size, const char *file, const int line) {
    printf("%s:%d: realloc(%p", file, line, ptr);
    void *rptr = (realloc)(ptr, size);
    printf(", %d)=%p\n", (int)size, rptr);
    return rptr;
}

/**
 @brief Debugging wrapper for calloc(size_t num, size_t size)
 
 @param[in] num Number of elements to allocate
 @param[in] size Size of each element
 @param[in] file Calling file
 @param[in] line Calling line
 @return A pointer to the allocated memory block on success, NULL on failure
 */
void *debug_calloc(const size_t num, const size_t size, const char *file, const int line) {
    void *ptr = (calloc)(num, size);
    printf("%s:%d: calloc(%d, %d)=%p\n", file, line, (int)num, (int)size, ptr);
    return ptr;
}
