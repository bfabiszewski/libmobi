//
//  compression.h
//  mobi
//
//  Created by Bartek on 27.03.14.
//  Copyright (c) 2014 Bartek. All rights reserved.
//

#ifndef mobi_lz77_h
#define mobi_lz77_h

#include <stdlib.h>
#include <stdint.h>

typedef struct {
    size_t index_count;
    size_t index_read;
    size_t code_length;
    uint32_t table1[256];
    uint32_t mincode_table[33];
    uint32_t maxcode_table[33];
    uint16_t *symbol_offsets;
    char **symbols;
} MOBIHuffCdic;

size_t mobi_decompress_lz77(char *out, const char *in, size_t len);
size_t mobi_decompress_huffman(char *out, const char *in, size_t len, MOBIHuffCdic *huffcdic, size_t depth);

#endif
