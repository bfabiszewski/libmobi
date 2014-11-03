/** @file buffer.c
 *  @brief Functions to read/write raw big endian data
 *
 * Copyright (c) 2014 Bartek Fabiszewski
 * http://www.fabiszewski.net
 *
 * This file is part of libmobi.
 * Licensed under LGPL, either version 3, or any later.
 * See <http://www.gnu.org/licenses/>
 */

#include <stdlib.h>
#include <string.h>
#include "buffer.h"
#include "debug.h"

/**
 @brief Initializer for MOBIBuffer structure
 
 It allocates memory for structure and for data.
 Memory should be freed with buffer_free().
 
 @param[in] len Size of data to be allocated for the buffer
 @return MOBIBuffer on success, NULL otherwise
 */
MOBIBuffer * buffer_init(const size_t len) {
    MOBIBuffer *buf = NULL;
    buf = malloc(sizeof(MOBIBuffer));
	if (buf == NULL) {
        debug_print("%s", "Buffer allocation failed\n");
        return NULL;
    }
    buf->data = malloc(len);
	if (buf->data == NULL) {
		free(buf);
        debug_print("%s", "Buffer data allocation failed\n");
		return NULL;
	}
	buf->offset = 0;
	buf->maxlen = len;
    buf->error = MOBI_SUCCESS;
	return buf;
}

/**
 @brief Initializer for MOBIBuffer structure
 
 It allocates memory for structure but, unlike buffer_init(), it does not allocate memory for data.
 Memory should be freed with buffer_free_null().
 
 @param[in] len Size of data held by the buffer
 @return MOBIBuffer on success, NULL otherwise
 */
MOBIBuffer * buffer_init_null(const size_t len) {
    MOBIBuffer *buf = NULL;
    buf = malloc(sizeof(MOBIBuffer));
	if (buf == NULL) {
        debug_print("%s", "Buffer allocation failed\n");
        return NULL;
    }
    buf->data = NULL;
	buf->offset = 0;
	buf->maxlen = len;
    buf->error = MOBI_SUCCESS;
	return buf;
}

/**
 @brief Adds 8-bit value to MOBIBuffer
 
 @param[in,out] buf MOBIBuffer structure to be filled with data
 @param[in] data Integer to be put into the buffer
 */
void buffer_add8(MOBIBuffer *buf, const uint8_t data) {
    if (buf->offset + 1 > buf->maxlen) {
        debug_print("%s", "Buffer full\n");
        buf->error = MOBI_BUFFER_END;
        return;
    }
	buf->data[buf->offset++] = data;
}

/**
 @brief Adds 16-bit value to MOBIBuffer
 
 @param[in,out] buf MOBIBuffer structure to be filled with data
 @param[in] data Integer to be put into the buffer
 */
void buffer_add16(MOBIBuffer *buf, const uint16_t data) {
    if (buf->offset + 2 > buf->maxlen) {
        debug_print("%s", "Buffer full\n");
        buf->error = MOBI_BUFFER_END;
        return;
    }
    unsigned char *buftr = buf->data + buf->offset;
    *buftr++ = (uint8_t)((uint32_t)(data & 0xff00U) >> 8);
    *buftr = (uint8_t)((uint32_t)(data & 0xffU));
    buf->offset += 2;
}

/**
 @brief Adds 32-bit value to MOBIBuffer
 
 @param[in,out] buf MOBIBuffer structure to be filled with data
 @param[in] data Integer to be put into the buffer
 */
void buffer_add32(MOBIBuffer *buf, const uint32_t data) {
    if (buf->offset + 4 > buf->maxlen) {
        debug_print("%s", "Buffer full\n");
        buf->error = MOBI_BUFFER_END;
        return;
    }
    unsigned char *buftr = buf->data + buf->offset;
    *buftr++ = (uint8_t)((uint32_t)(data & 0xff000000U) >> 16);
    *buftr++ = (uint8_t)((uint32_t)(data & 0xff0000U) >> 12);
    *buftr++ = (uint8_t)((uint32_t)(data & 0xff00U) >> 8);
    *buftr = (uint8_t)((uint32_t)(data & 0xffU));
    buf->offset += 4;
}

/**
 @brief Adds raw data to MOBIBuffer
 
 @param[in,out] buf MOBIBuffer structure to be filled with data
 @param[in] data Pointer to read data
 @param[in] len Size of the read data
 */
void buffer_addraw(MOBIBuffer *buf, const unsigned char* data, const size_t len) {
    if (buf->offset + len > buf->maxlen) {
        debug_print("%s", "Buffer full\n");
        buf->error = MOBI_BUFFER_END;
        return;
    }
    memcpy(buf->data + buf->offset, data, len);
    buf->offset += len;
}

/**
 @brief Adds zero padded string to MOBIBuffer
 
 @param[in,out] buf MOBIBuffer structure to be filled with data
 @param[in] str Pointer to string
 */
void buffer_addstring(MOBIBuffer *buf, const char *str) {
    const size_t len = strlen(str);
    buffer_addraw(buf, (const unsigned char *) str, len);
}

/**
 @brief Adds count of zeroes to MOBIBuffer
 
 @param[in,out] buf MOBIBuffer structure to be filled with data
 @param[in] count Number of zeroes to be put into the buffer
 */
void buffer_addzeros(MOBIBuffer *buf, const size_t count) {
    if (buf->offset + count > buf->maxlen) {
        debug_print("%s", "Buffer full\n");
        buf->error = MOBI_BUFFER_END;
        return;
    }
    memset(buf->data + buf->offset, 0, count);
    buf->offset += count;
}

/**
 @brief Reads 8-bit value from MOBIBuffer
 
 @param[in] buf MOBIBuffer structure containing data
 @return Read value, 0 if end of buffer is encountered
 */
uint8_t buffer_get8(MOBIBuffer *buf) {
    if (buf->offset + 1 > buf->maxlen) {
        debug_print("%s", "End of buffer\n");
        buf->error = MOBI_BUFFER_END;
        return 0;
    }
    return buf->data[buf->offset++];
}

/**
 @brief Reads 16-bit value from MOBIBuffer
 
 @param[in] buf MOBIBuffer structure containing data
 @return Read value, 0 if end of buffer is encountered
 */
uint16_t buffer_get16(MOBIBuffer *buf) {
    if (buf->offset + 2 > buf->maxlen) {
        debug_print("%s", "End of buffer\n");
        buf->error = MOBI_BUFFER_END;
        return 0;
    }
    uint16_t val;
    val = (uint16_t)((uint16_t) buf->data[buf->offset] << 8 | (uint16_t) buf->data[buf->offset + 1]);
    buf->offset += 2;
    return val;
}

/**
 @brief Reads 32-bit value from MOBIBuffer
 
 @param[in] buf MOBIBuffer structure containing data
 @return Read value, 0 if end of buffer is encountered
 */
uint32_t buffer_get32(MOBIBuffer *buf) {
    if (buf->offset + 4 > buf->maxlen) {
        debug_print("%s", "End of buffer\n");
        buf->error = MOBI_BUFFER_END;
        return 0;
    }
    uint32_t val;
    val = (uint32_t) buf->data[buf->offset] << 24 | (uint32_t) buf->data[buf->offset + 1] << 16 | (uint32_t) buf->data[buf->offset + 2] << 8 | (uint32_t) buf->data[buf->offset + 3];
    buf->offset += 4;
    return val;
}

/**
 @brief Reads variable length value from MOBIBuffer
 
 Internal function for wrappers: 
 buffer_get_varlen();
 buffer_get_varlen_dec();
 
 Reads maximum 4 bytes from the buffer. Stops when byte has bit 7 set.
 
 @param[in] buf MOBIBuffer structure containing data
 @param[out] len Value will be increased by number of bytes read
 @param[in] direction 1 - read buffer forward, -1 - read buffer backwards
 @return Read value, 0 if end of buffer is encountered
 */
static uint32_t _buffer_get_varlen(MOBIBuffer *buf, size_t *len, const int direction) {
    uint32_t val = 0;
    uint8_t byte_count = 0;
    uint8_t byte;
    uint8_t stop_flag = 0x80U;
    uint8_t mask = 0x7fU;
    do {
        if (direction == 1) {
            if (buf->offset + 1 > buf->maxlen) {
                debug_print("%s", "End of buffer\n");
                buf->error = MOBI_BUFFER_END;
                return val;
            }
            byte = buf->data[buf->offset++];
        } else {
            if (buf->offset < 1) {
                debug_print("%s", "End of buffer\n");
                buf->error = MOBI_BUFFER_END;
                return val;
            }
            byte = buf->data[buf->offset--];
        }
        val <<= 7;
        val |= (byte & mask);
        (*len)++;
        byte_count++;
    } while (!(byte & stop_flag) && (byte_count < 4));
    return val;
}

/**
 @brief Reads variable length value from MOBIBuffer
 
 Reads maximum 4 bytes from the buffer. Stops when byte has bit 7 set.
 
 @param[in] buf MOBIBuffer structure containing data
 @param[out] len Value will be increased by number of bytes read
 @return Read value, 0 if end of buffer is encountered
 */
uint32_t buffer_get_varlen(MOBIBuffer *buf, size_t *len) {
    return _buffer_get_varlen(buf, len, 1);
}

/**
 @brief Reads variable length value from MOBIBuffer going backwards
 
 Reads maximum 4 bytes from the buffer. Stops when byte has bit 7 set.
 
 @param[in] buf MOBIBuffer structure containing data
 @param[out] len Value will be increased by number of bytes read
 @return Read value, 0 if end of buffer is encountered
 */
uint32_t buffer_get_varlen_dec(MOBIBuffer *buf, size_t *len) {
    return _buffer_get_varlen(buf, len, -1);
}

/**
 @brief Reads raw data from MOBIBuffer and pads it with zero character
 
 @param[out] str Destination for string read from buffer. Length must be (len + 1)
 @param[in] buf MOBIBuffer structure containing data
 @param[in] len Length of the data to be read from buffer
 */
void buffer_getstring(char *str, MOBIBuffer *buf, const size_t len) {
    if (!str) {
        buf->error = MOBI_PARAM_ERR;
        return;
    }
    if (buf->offset + len > buf->maxlen) {
        debug_print("%s", "End of buffer\n");
        buf->error = MOBI_BUFFER_END;
        return;
    }
    memcpy(str, buf->data + buf->offset, len);
    str[len] = '\0';
    buf->offset += len;
}

/**
 @brief Reads raw data from MOBIBuffer and pads it with zero character
 
 FIXME: This function skips zeroes while reading from buffer,
 as (malformed?) orth index tag labels sometimes contain null characters.
 
 @param[out] str Destination for string read from buffer. Length must be (len + 1)
 @param[in] buf MOBIBuffer structure containing data
 @param[in] len Length of the data to be read from buffer
 */
size_t buffer_getstring_skipzeroes(char *str, MOBIBuffer *buf, const size_t len) {
    if (!str) {
        buf->error = MOBI_PARAM_ERR;
        return 0;
    }
    if (buf->offset + len > buf->maxlen) {
        debug_print("%s", "End of buffer\n");
        buf->error = MOBI_BUFFER_END;
        return 0;
    }
    size_t in = 0;
    size_t out = 0;
    while (out < len) {
        if (buf->data[buf->offset + out] != 0) {
            str[in++] = (char) buf->data[buf->offset + out];
        }
        out++;
    }
    str[in] = '\0';
    buf->offset += len;
    return in;
}

/**
 @brief Reads raw data from MOBIBuffer, appends it to a string and pads it with zero character
 
 @param[in,out] str A string to which data will be appended
 @param[in] buf MOBIBuffer structure containing data
 @param[in] len Length of the data to be read from buffer
 */
void buffer_appendstring(char *str, MOBIBuffer *buf, const size_t len) {
    if (!str) {
        buf->error = MOBI_PARAM_ERR;
        return;
    }
    if (buf->offset + len > buf->maxlen) {
        debug_print("%s", "End of buffer\n");
        buf->error = MOBI_BUFFER_END;
        return;
    }
    size_t str_len = strlen(str);
    memcpy(str + str_len, buf->data + buf->offset, len);
    str[str_len + len] = '\0';
    buf->offset += len;
}

/**
 @brief Reads raw data from MOBIBuffer
 
 @param[out] data Destination to which data will be appended
 @param[in] buf MOBIBuffer structure containing data
 @param[in] len Length of the data to be read from buffer
 */
void buffer_getraw(void *data, MOBIBuffer *buf, const size_t len) {
    if (!data) {
        buf->error = MOBI_PARAM_ERR;
        return;
    }
    if (buf->offset + len > buf->maxlen) {
        debug_print("%s", "End of buffer\n");
        buf->error = MOBI_BUFFER_END;
        return;
    }
    memcpy(data, buf->data + buf->offset, len);
    buf->offset += len;
}

/**
 @brief Read 8-bit value from MOBIBuffer into allocated memory
 
 Read 8-bit value from buffer into memory allocated by the function.
 Returns pointer to the value, which must be freed later.
 If the data is not accessible function will return null pointer.
 
 @param[out] val Pointer to value or null pointer on failure
 @param[in] buf MOBIBuffer structure containing data
 */
void buffer_dup8(uint8_t **val, MOBIBuffer *buf) {
    *val = NULL;
    if (buf->offset + 1 > buf->maxlen) {
        return;
    }
    *val = malloc(sizeof(uint8_t));
    if (*val == NULL) {
        return;
    }
    **val = buffer_get8(buf);
}

/**
 @brief Read 16-bit value from MOBIBuffer into allocated memory
 
 Read 16-bit value from buffer into allocated memory.
 Returns pointer to the value, which must be freed later.
 If the data is not accessible function will return null pointer.
 
 @param[out] val Pointer to value or null pointer on failure
 @param[in] buf MOBIBuffer structure containing data
 */
void buffer_dup16(uint16_t **val, MOBIBuffer *buf) {
    *val = NULL;
    if (buf->offset + 2 > buf->maxlen) {
        return;
    }
    *val = malloc(sizeof(uint16_t));
    if (*val == NULL) {
        return;
    }
    **val = buffer_get16(buf);
}

/**
 @brief Read 32-bit value from MOBIBuffer into allocated memory
 
 Read 32-bit value from buffer into allocated memory.
 Returns pointer to the value, which must be freed later.
 If the data is not accessible function will return null pointer.
 
 @param[out] val Pointer to value
 @param[in] buf MOBIBuffer structure containing data
 */
void buffer_dup32(uint32_t **val, MOBIBuffer *buf) {
    *val = NULL;
    if (buf->offset + 4 > buf->maxlen) {
        return;
    }
    *val = malloc(sizeof(uint32_t));
    if (*val == NULL) {
        return;
    }
    **val = buffer_get32(buf);
}

/**
 @brief Copy 8-bit value from one MOBIBuffer into another
 
 @param[out] dest Destination buffer
 @param[in] source Source buffer
 */
void buffer_copy8(MOBIBuffer *dest, MOBIBuffer *source) {
    buffer_add8(dest, buffer_get8(source));
}

/**
 @brief Copy raw value from one MOBIBuffer into another
 
 @param[out] dest Destination buffer
 @param[in] source Source buffer
 @param[in] len Number of bytes to copy
 */
void buffer_copy(MOBIBuffer *dest, MOBIBuffer *source, const size_t len) {
    if (source->offset + len > source->maxlen) {
        debug_print("%s", "End of buffer\n");
        source->error = MOBI_BUFFER_END;
        return;
    }
    if (dest->offset + len > dest->maxlen) {
        debug_print("%s", "End of buffer\n");
        dest->error = MOBI_BUFFER_END;
        return;
    }
    memcpy(dest->data + dest->offset, source->data + source->offset, len);
    dest->offset += len;
    source->offset += len;
}

/**
 @brief Check if buffer data header contains magic signature
 
 @param[in] buf MOBIBuffer buffer containing data
 @param[in] magic Magic signature
 @return boolean true on match, false otherwise
 */
bool buffer_match_magic(MOBIBuffer *buf, const char *magic) {
    const size_t magic_length = strlen(magic);
    if (buf->offset + magic_length > buf->maxlen) {
        return false;
    }
    if (memcmp(buf->data + buf->offset, magic, magic_length) == 0) {
        return true;
    }
    return false;
}

/**
 @brief Move current buffer offset by diff bytes
 
 @param[in,out] buf MOBIBuffer buffer containing data
 @param[in] diff Number of bytes by which the offset is adjusted
 */
void buffer_seek(MOBIBuffer *buf, const int diff) {
    size_t adiff = (size_t) abs(diff);
    if (diff >= 0) {
        if (buf->offset + adiff <= buf->maxlen) {
            buf->offset += adiff;
            return;
        }
    } else {
        if (buf->offset >= adiff) {
            buf->offset -= adiff;
            return;
        }
    }
    buf->error = MOBI_BUFFER_END;
    debug_print("%s", "End of buffer\n");
}

/**
 @brief Set buffer offset to pos position
 
 @param[in,out] buf MOBIBuffer buffer containing data
 @param[in] pos New position
 */
void buffer_setpos(MOBIBuffer *buf, const size_t pos) {
    if (pos <= buf->maxlen) {
        buf->offset = pos;
        return;
    }
    buf->error = MOBI_BUFFER_END;
    debug_print("%s", "End of buffer\n");
}

/**
 @brief Free pointer to MOBIBuffer structure and pointer to data
 
 Free data initialized with buffer_init();
 
 @param[in] buf MOBIBuffer structure
 */
void buffer_free(MOBIBuffer *buf) {
	if (buf == NULL) { return; }
	if (buf->data != NULL) {
		free(buf->data);
	}
	free(buf);
}

/**
 @brief Free pointer to MOBIBuffer structure
 
 Free data initialized with buffer_init_null();
 Unlike buffer_free() it will not free pointer to buf->data
 
 @param[in] buf MOBIBuffer structure
 */
void buffer_free_null(MOBIBuffer *buf) {
	if (buf == NULL) { return; }
	free(buf);
}

/**
 @brief Initializer for MOBIArray structure
 
 It allocates memory for structure and for data: array of uint32_t variables.
 Memory should be freed with array_free().
 
 @param[in] len Initial size of the array
 @return MOBIArray on success, NULL otherwise
 */
MOBIArray * array_init(const size_t len) {
    MOBIArray *arr = NULL;
    arr = malloc(sizeof(MOBIArray));
    if (arr == NULL) {
        debug_print("%s", "Array allocation failed\n");
        return NULL;
    }
    arr->data = malloc(len * sizeof(*arr->data));
	if (arr->data == NULL) {
		free(arr);
        debug_print("%s", "Array data allocation failed\n");
		return NULL;
	}
    arr->maxsize = arr->step = len;
    arr->size = 0;
    return arr;
}

/**
 @brief Inserts value into MOBIArray array
 
 @param[in,out] arr MOBIArray array
 @param[in] value Value to be inserted
 @return MOBI_RET status code (on success MOBI_SUCCESS)
 */
MOBI_RET array_insert(MOBIArray *arr, const uint32_t value) {
    if (!arr || arr->maxsize == 0) {
        return MOBI_INIT_FAILED;
    }
    if (arr->maxsize == arr->size) {
        arr->maxsize += arr->step;
        uint32_t *tmp = realloc(arr->data, arr->maxsize * sizeof(*arr->data));
        if (!tmp) {
            free(arr->data);
            arr->data = NULL;
            return MOBI_MALLOC_FAILED;
        }
        arr->data = tmp;
    }
    arr->data[arr->size] = value;
    arr->size++;
    return MOBI_SUCCESS;
}

/**
 @brief Helper for qsort in array_sort() function.
 
 @param[in] a First element to compare
 @param[in] b Second element to compare
 @return -1 if a < b; 1 if a > b; 0 if a = b
 */
static int array_compare(const void *a, const void *b) {
    if (*(uint32_t *) a < *(uint32_t *) b) {
        return -1;
    };
    if (*(uint32_t *) a > *(uint32_t *) b) {
        return 1;
    };
    return 0;
}

/**
 @brief Sort MOBIArray in ascending order.
 
 When unique is set to true, duplicate values are discarded.
 
 @param[in,out] arr MOBIArray array
 @param[in] unique Discard duplicate values if true
 */
void array_sort(MOBIArray *arr, const bool unique) {
    if (!arr || !arr->data || arr->size == 0) {
        return;
    }
    qsort(arr->data, arr->size, sizeof(*arr->data), array_compare);
    if (unique) {
        size_t i = 1, j = 1;
        while (i < arr->size) {
            if (arr->data[j - 1] == arr->data[i]) {
                i++;
                continue;
            }
            arr->data[j++] = arr->data[i++];
        }
        arr->size = j;
    }
}

/**
 @brief Get size of the array
 
 @param[in] arr MOBIArray structure
 */
size_t array_size(MOBIArray *arr) {
    return arr->size;
}

/**
 @brief Free MOBIArray structure and contained data
 
 Free data initialized with array_init();
 
 @param[in] arr MOBIArray structure
 */
void array_free(MOBIArray *arr) {
    if (!arr) { return; }
    if (arr->data) {
        free(arr->data);
    }
    free(arr);
}
