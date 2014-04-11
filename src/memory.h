//
//  memory.h
//  mobi
//
//  Created by Bartek on 31.03.14.
//  Copyright (c) 2014 Bartek. All rights reserved.
//

#ifndef mobi_memory_h
#define mobi_memory_h

#include "mobi.h"

MOBIData * mobi_init();
void mobi_free_mh(MOBIData *m);
void mobi_free_rec(MOBIData *m);
void mobi_free_eh(MOBIData *m);
void mobi_free(MOBIData *m);

MOBIHuffCdic * mobi_init_huffcdic(MOBIData *m);
void mobi_free_huffcdic(MOBIHuffCdic *huffcdic);

#endif
