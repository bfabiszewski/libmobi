//
//  read.h
//  mobi
//
//  Created by Bartek on 26.03.14.
//  Copyright (c) 2014 Bartek. All rights reserved.
//

#ifndef mobi_read_h
#define mobi_read_h

#include "mobi.h"
#include "memory.h"
#include "util.h"

int mobi_load_pdbheader(MOBIData *m, FILE *file);
int mobi_load_reclist(MOBIData *m, FILE *file);
int mobi_load_recdata(MOBIData *m, FILE *file);
int mobi_load_rec(MOBIPdbRecord *rec, FILE *file);

#endif
