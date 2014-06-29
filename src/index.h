/** @file index.h
 *
 * Copyright (c) 2014 Bartek Fabiszewski
 * http://www.fabiszewski.net
 *
 * This file is part of libmobi.
 * Licensed under LGPL, either version 3, or any later.
 * See <http://www.gnu.org/licenses/>
 */

#ifndef mobi_index_h
#define mobi_index_h

#include "config.h"
#include "buffer.h"
#include "mobi.h"

/**
 @defgroup index_tag Predefined tag arrays: {tagid, tagindex} for mobi_get_indxentry_tagvalue()
 @{
 */
#define INDX_TAG_GUIDE_TITLE_CNCX (unsigned[]) {1, 0} /**< Guide title CNCX offset */

#define INDX_TAG_NCX_FILEPOS (unsigned[]) {1, 0} /**< NCX filepos offset */
#define INDX_TAG_NCX_TEXT_CNCX (unsigned[]) {3, 0} /**< NCX text CNCX offset */
#define INDX_TAG_NCX_LEVEL (unsigned[]) {4, 0} /**< NCX level */
#define INDX_TAG_NCX_KIND_CNCX (unsigned[]) {5, 0} /**< NCX kind CNCX offset */
#define INDX_TAG_NCX_POSFID (unsigned[]) {6, 0} /**< NCX pos:fid */
#define INDX_TAG_NCX_POSOFF (unsigned[]) {6, 1} /**< NCX pos:off */
#define INDX_TAG_NCX_PARENT (unsigned[]) {21, 0} /**< NCX parent */
#define INDX_TAG_NCX_CHILD_START (unsigned[]) {22, 0} /**< NCX start child */
#define INDX_TAG_NCX_CHILD_END (unsigned[]) {23, 0} /**< NCX last child */

#define INDX_TAG_SKEL_COUNT (unsigned[]) {1, 0} /**< Skel fragments count */
#define INDX_TAG_SKEL_POSITION (unsigned[]) {6, 0} /**< Skel position */
#define INDX_TAG_SKEL_LENGTH (unsigned[]) {6, 1} /**< Skel length */

#define INDX_TAG_FRAG_AID_CNCX (unsigned[]) {2, 0} /**< Frag aid CNCX offset */
#define INDX_TAG_FRAG_FILE_NR (unsigned[]) {3, 0} /**< Frag file number */
#define INDX_TAG_FRAG_SEQUENCE_NR (unsigned[]) {4, 0} /**< Frag sequence number */
#define INDX_TAG_FRAG_POSITION (unsigned[]) {6, 0} /**< Frag position */
#define INDX_TAG_FRAG_LENGTH (unsigned[]) {6, 1} /**< Frag length */
/** @} */

/**
 @brief Tag entries in TAGX section (for internal INDX parsing)
 */
typedef struct {
    uint8_t tag; /**< Tag */
    uint8_t values_count; /**< Number of values */
    uint8_t bitmask; /**< Bitmask */
    uint8_t control_byte; /**< EOF control byte */
} TAGXTags;

/**
 @brief Parsed TAGX section (for internal INDX parsing)
 
 TAGX tags hold metadata of index entries.
 It is present in the first index record.
 */
typedef struct {
    TAGXTags *tags; /**< Array of tag entries */
    size_t tags_count; /**< Number of tag entries */
    size_t control_byte_count; /**< Number of control bytes */
} MOBITagx;

/**
 @brief Parsed IDXT section (for internal INDX parsing)
 
 IDXT section holds offsets to index entries
 */
typedef struct {
    uint32_t *offsets; /**< Offsets to index entries */
    size_t offsets_count; /**< Offsets count */
} MOBIIdxt;

MOBI_RET mobi_parse_indx(const MOBIPdbRecord *indx_record, MOBIIndx *indx, MOBITagx *tagx);
MOBI_RET mobi_get_indxentry_tagvalue(uint32_t *tagvalue, const MOBIIndexEntry *entry, const unsigned tag_arr[]);
char * mobi_get_cncx_string(const MOBIPdbRecord *cncx_record, const uint32_t cncx_offset);
#endif
