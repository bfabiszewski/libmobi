//
//  compression.c
//  mobi
//
//  Created by Bartek on 27.03.14.
//  Copyright (c) 2014 Bartek. All rights reserved.
//

#include <string.h>
#include <stdio.h>

#include "compression.h"
#include "mobi.h"


// PalmDOC version of LZ77 compression
// Decompressor based on this algorithm:
// http://en.wikibooks.org/wiki/Data_Compression/Dictionary_compression#PalmDoc
//
size_t mobi_decompress_lz77(char *out, const char *in, size_t len) {
    size_t start_in = (size_t) in;
    size_t start_out = (size_t) out;
    while ((size_t) in - start_in < len) {
        uint8_t val = (uint8_t) in[0];
        // byte pair: space + char
        if (val >= 0xc0) {
            *(out++) = ' ';
            *(out++) = val ^ 0x80;
            in++;
        }
        // length, distance pair
        // 0x8000 + (distance << 3) + ((length-3) & 0x07)
        else if (val >= 0x80) {
            uint16_t distance = ((((in[0] << 8) | ((uint8_t)in[1])) >> 3) & 0x7ff);
            uint8_t length = (in[1] & 0x7) + 3;
            while (length-- > 0) {
                *(out) = *(out - distance);
                out++;
            }
            in += 2;
        }
        // single char, not modified
        else if (val >= 0x09) {
            *(out++) = *(in++);
        }
        // n chars not modified
        else if (val >= 0x01) {
            memcpy(out, ++in, val);
            out += val;
            in += val;
        }
        // char '\0', not modified
        else {
            *(out++) = *(in++);
        }
    }
    return (size_t) out - start_out;
}

uint64_t _fill_buffer(const char *in, size_t len) {
    uint32_t in1 = 0L;
    uint32_t in2 = 0L;
    len = (len < 8) ? len : 8;
    size_t i = 0;
    while (i < len && i < 4) {
        in1 |= (uint8_t) in[i] << ((3-i) * 8);
        i++;
    }
    while (i < len) {
        in2 |= (uint8_t) in[i] << ((3-i) * 8);
        i++;
    }
    return (uint64_t) in1 << 32 | in2;
}

int shortcnt = 0;

// Mobi version of Huffman coding
// Decompressor and HUFF/CDIC records parsing based on:
// perl EBook::Tools::Mobipocket
// python mobiunpack.py, calibre
size_t mobi_decompress_huffman(char *out, const char *in, size_t len, MOBIHuffCdic *huffcdic, size_t depth) {
    size_t start_out = (size_t) out;
    int8_t bitcount = 32;
    int32_t bitsleft = (int32_t) len * 8;
    uint32_t t1, offset;
    uint32_t code, maxcode, symbol_length;
    uint8_t code_length = 0, i;
    uint32_t index;
    uint64_t buffer;
    buffer = _fill_buffer(in, len);
    while (1) {
        if (bitcount <= 0) {
            bitcount += 32;
            in += 4;
            buffer = _fill_buffer(in, (bitsleft + (8 - 1)) / 8);
        }
        code = (buffer >> bitcount) & 0xffffffff;
        // lookup code in table1
        t1 = huffcdic->table1[code >> 24];
        // get maxcode and codelen from t1
        code_length = t1 & 0x1f;
        maxcode = (((t1 >> 8) + 1) << (32 - code_length)) - 1;
        // check termination bit
        if (!(t1 & 0x80)) {
            // get offset from mincode, maxcode tables
            while (code < huffcdic->mincode_table[code_length]) {
                code_length++;
            }
            maxcode = huffcdic->maxcode_table[code_length];
        }
        bitcount -= code_length;
        bitsleft -= code_length;
        if (bitsleft < 0) {
            break;
        }
        // get index for symbol offset
        index = (maxcode - code) >> (32 - code_length);
        // check which part of cdic to use
        i = index >> huffcdic->code_length;
        // get offset
        offset = huffcdic->symbol_offsets[index];
        symbol_length = (uint8_t) huffcdic->symbols[i][offset] << 8 | (uint8_t) huffcdic->symbols[i][offset + 1];
        // 1st bit is is_decompressed flag
        int is_decompressed = symbol_length >> 15;
        // get rid of flag
        symbol_length &= 0x7fff;
        if (is_decompressed) {
            memcpy(out, (huffcdic->symbols[i] + offset + 2), symbol_length);
            out += symbol_length;
        } else {
            // symbol is compressed
            // TODO cache uncompressed symbols?
            out += mobi_decompress_huffman(out, (huffcdic->symbols[i] + offset + 2), (symbol_length), huffcdic, depth + 1);
        }
    }
    return (size_t) out - start_out;

}
