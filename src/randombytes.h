/** @file randombytes.h
 *
 * Copyright (c) 2021 Bartek Fabiszewski
 * http://www.fabiszewski.net
 *
 * This file is part of libmobi.
 * Licensed under LGPL, either version 3, or any later.
 * See <http://www.gnu.org/licenses/>
 */

#ifndef libmobi_randombytes_h
#define libmobi_randombytes_h

#ifdef _WIN32
# include <crtdefs.h>
#else
# include <unistd.h>
#endif /* _WIN32 */

#include "config.h"

#if (MOBI_DEBUG)
//# define RANDOMBYTES_DEBUG
#endif

/**
 @brief Write n random bytes of high quality to buf
 
 @param[in,out] buf Buffer to be filled with random bytes
 @param[in] len Buffer length
 @return On success returns 0
 */
int randombytes(void *buf, size_t len);

#endif
