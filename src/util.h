//
//  util.h
//  mobi
//
//  Created by Bartek on 08.04.14.
//  Copyright (c) 2014 Bartek. All rights reserved.
//

#ifndef mobi_util_h
#define mobi_util_h

#include "mobi.h"
#include "memory.h"

int mobi_delete_record_by_seqnumber(MOBIData *m, size_t num);
int mobi_swap_mobidata(MOBIData *m);
#endif
