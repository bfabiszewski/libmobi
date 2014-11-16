/** @file util.h
 *
 * Copyright (c) 2014 Bartek Fabiszewski
 * http://www.fabiszewski.net
 *
 * This file is part of libmobi.
 * Licensed under LGPL, either version 3, or any later.
 * See <http://www.gnu.org/licenses/>
 */

#ifndef libmobi_util_h
#define libmobi_util_h

#include "config.h"
#include "mobi.h"
#include "memory.h"
#include "buffer.h"
#include "compression.h"

#ifndef HAVE_STRDUP
/** @brief strdup replacement */
#define strdup mobi_strdup
#endif

#ifdef USE_MINIZ
#include "miniz.h"
#define m_uncompress mz_uncompress
#define M_OK MZ_OK
#else
#include <zlib.h>
#define m_uncompress uncompress
#define M_OK Z_OK
#endif

/** @brief Magic numbers of records */
#define MOBI_MAGIC "MOBI"
#define EXTH_MAGIC "EXTH"
#define HUFF_MAGIC "HUFF"
#define CDIC_MAGIC "CDIC"
#define FDST_MAGIC "FDST"
#define IDXT_MAGIC "IDXT"
#define INDX_MAGIC "INDX"
#define LIGT_MAGIC "LIGT"
#define ORDT_MAGIC "ORDT"
#define TAGX_MAGIC "TAGX"
#define FONT_MAGIC "FONT"
#define AUDI_MAGIC "AUDI"
#define VIDE_MAGIC "VIDE"
#define BOUNDARY_MAGIC "BOUNDARY"
#define EOF_MAGIC "\xe9\x8e\r\n"
#define REPLICA_MAGIC "%MOP"

/** @brief Difference in seconds between epoch time and mac time */
#define EPOCH_MAC_DIFF 2082844800UL

/** 
 @defgroup mobi_pdb Params for pdb record header structure
 @{
 */
#define PALMDB_HEADER_LEN 78 /**< Length of header without record info headers */
#define PALMDB_NAME_SIZE_MAX 32 /**< Max length of db name stored at offset 0 */
#define PALMDB_RECORD_INFO_SIZE 8 /**< Record info header size of each pdb record */
/** @} */

/** 
 @defgroup mobi_pdb_defs Default values for pdb record header structure
 @{
 */
#define PALMDB_ATTRIBUTE_DEFAULT 0
#define PALMDB_VERSION_DEFAULT 0
#define PALMDB_MODNUM_DEFAULT 0
#define PALMDB_APPINFO_DEFAULT 0
#define PALMDB_SORTINFO_DEFAULT 0
#define PALMDB_TYPE_DEFAULT "BOOK"
#define PALMDB_CREATOR_DEFAULT "MOBI"
#define PALMDB_NEXTREC_DEFAULT 0
/** @} */

/** 
 @defgroup mobi_rec0 Params for record0 header structure
 @{ 
 */
#define RECORD0_HEADER_LEN 16 /**< Length of Record 0 header */
#define RECORD0_NO_COMPRESSION 1 /**< Text record compression type: none */
#define RECORD0_PALMDOC_COMPRESSION 2 /**< Text record compression type: palmdoc */
#define RECORD0_HUFF_COMPRESSION 17480 /**< Text record compression type: huff/cdic */
#define RECORD0_TEXT_SIZE_MAX 4096 /**< Max size of uncompressed text record */
#define RECORD0_NO_ENCRYPTION 0 /**< Text record encryption type: none */
#define RECORD0_OLD_ENCRYPTION 1 /**< Text record encryption type: old mobipocket */
#define RECORD0_MOBI_ENCRYPTION 2 /**< Text record encryption type: mobipocket */
/** @} */

/** 
 @defgroup mobi_len Header length / size of records 
 @{ 
 */
#define CDIC_HEADER_LEN 16
#define HUFF_HEADER_LEN 24
#define HUFF_RECORD_MINSIZE 2584
#define FONT_HEADER_LEN 24
#define MEDIA_HEADER_LEN 12
/** @} */

/**
 @defgroup mobi_return Values returned by functions
 @{
 */
#define MOBI_NOTSET UINT32_MAX /**< Value is not set */
/** @} */

#define max(a, b) ((a) > (b) ? (a) : (b))
#define min(a, b) ((a) < (b) ? (a) : (b))

int mobi_bitcount(uint8_t byte);
MOBI_RET mobi_delete_record_by_seqnumber(MOBIData *m, size_t num);
MOBI_RET mobi_swap_mobidata(MOBIData *m);
char * mobi_strdup(const char *s);
bool mobi_is_cp1252(const MOBIData *m);
MOBI_RET mobi_cp1252_to_utf8(char *output, const char *input, size_t *outsize, const size_t insize);
uint8_t mobi_ligature_to_cp1252(const uint8_t c1, const uint8_t c2);
uint16_t mobi_ligature_to_utf16(const uint32_t control, const uint32_t c);
MOBIPart * mobi_get_part_by_uid(const MOBIRawml *rawml, const size_t uid);
size_t mobi_get_first_resource_record(const MOBIData *m);
MOBIFiletype mobi_determine_resource_type(const MOBIPdbRecord *record);
MOBIFiletype mobi_determine_flowpart_type(const MOBIRawml *rawml, const size_t part_number);
MOBI_RET mobi_base32_decode(uint32_t *decoded, const char *encoded);
MOBIPart * mobi_get_flow_by_uid(const MOBIRawml *rawml, const size_t uid);
MOBIPart * mobi_get_resource_by_uid(const MOBIRawml *rawml, const size_t uid);
MOBIFiletype mobi_get_resourcetype_by_uid(const MOBIRawml *rawml, const size_t uid);
MOBI_RET mobi_add_audio_resource(MOBIPart *part);
MOBI_RET mobi_add_video_resource(MOBIPart *part);
MOBI_RET mobi_add_font_resource(MOBIPart *part);
#endif
