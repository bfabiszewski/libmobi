/** @file compression.h
 *
 * Copyright (c) 2014 Bartek Fabiszewski
 * http://www.fabiszewski.net
 *
 * This file is part of libmobi.
 * Licensed under LGPL, either version 3, or any later.
 * See <http://www.gnu.org/licenses/>
 */

#ifndef libmobi_compression_h
#define libmobi_compression_h

#include "config.h"
#include "mobi.h"

#ifndef MOBI_INLINE
#define MOBI_INLINE /**< Syntax for compiler inline keyword from config.h */
#endif

/* FIXME: what is the reasonable value? */
#define MOBI_HUFFMAN_MAXDEPTH 15 /**< Maximal recursion level for huffman decompression routine */

MOBI_RET mobi_decompress_lz77(unsigned char *out, const unsigned char *in, size_t *len_out, const size_t len_in);
MOBI_RET mobi_decompress_huffman(unsigned char *out, const unsigned char *in, size_t *len_out, size_t len_in, const MOBIHuffCdic *huffcdic);

#endif
