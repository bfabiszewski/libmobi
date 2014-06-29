/** @file index.c
 *  @brief Functions to parse index records
 *
 * Copyright (c) 2014 Bartek Fabiszewski
 * http://www.fabiszewski.net
 *
 * This file is part of libmobi.
 * Licensed under LGPL, either version 3, or any later.
 * See <http://www.gnu.org/licenses/>
 */

#include <string.h>
#include <stdlib.h>
#include <stdint.h>

#include "index.h"
#include "util.h"
#include "memory.h"
#include "debug.h"

/**
 @brief Parser of TAGX section of INDX record
 
 @param[in,out] buf MOBIBuffer structure, offset pointing at beginning of TAGX section
 @param[in,out] tagx MOBITagx structure to be filled by the function
 @return MOBI_RET status code (on success MOBI_SUCCESS)
 */
static MOBI_RET mobi_parse_tagx(MOBIBuffer *buf, MOBITagx *tagx) {
    tagx->control_byte_count = 0;
    tagx->tags_count = 0;
    tagx->tags = NULL;
    buf->offset += 4; /* skip header */
    const uint32_t tagx_header_length = buffer_get32(buf);
    if (tagx_header_length < 16) {
        debug_print("INDX wrong header length: %u\n", tagx_header_length);
        return MOBI_DATA_CORRUPT;
    }
    tagx->control_byte_count = buffer_get32(buf);
    const size_t tagx_data_length = (tagx_header_length - 12) / 4;
    tagx->tags = malloc(tagx_header_length * sizeof(TAGXTags));
    if (tagx->tags == NULL) {
        debug_print("%s", "Memory allocation failed for TAGX tags\n");
        return MOBI_MALLOC_FAILED;
    }
    size_t i = 0;
    while (i < tagx_data_length) {
        tagx->tags[i].tag = buffer_get8(buf);
        tagx->tags[i].values_count = buffer_get8(buf);
        tagx->tags[i].bitmask = buffer_get8(buf);
        const uint8_t control_byte = buffer_get8(buf);
        tagx->tags[i].control_byte = control_byte;
        debug_print("tagx[%zu]:\t%i\t%i\t%i\t%i\n", i, tagx->tags[i].tag, tagx->tags[i].values_count, tagx->tags[i].bitmask, control_byte);
        i++;
    }
    tagx->tags_count = i;
    return MOBI_SUCCESS;
}

/**
 @brief Parser of IDXT section of INDX record
 
 @param[in,out] buf MOBIBuffer structure, offset pointing at beginning of TAGX section
 @param[in,out] idxt MOBITagx structure to be filled by the function
 @param[in] entries_count Number of index entries
 @return MOBI_RET status code (on success MOBI_SUCCESS)
 */
static MOBI_RET mobi_parse_idxt(MOBIBuffer *buf, MOBIIdxt *idxt, const size_t entries_count) {
    const uint32_t idxt_offset = (uint32_t) buf->offset;
    idxt->offsets_count = 0;
    char idxt_magic[5];
    buffer_getstring(idxt_magic, buf, 4);
    if (strncmp(idxt_magic, IDXT_MAGIC, 4) != 0) {
        debug_print("IDXT wrong magic: %s\n", idxt_magic);
        return MOBI_DATA_CORRUPT;
    }
    size_t i = 0;
    while (i < entries_count) {
        /* entry offsets */
        idxt->offsets[i++] = buffer_get16(buf);
    }
    /* last entry end position is IDXT tag offset */
    idxt->offsets[i] = idxt_offset;
    idxt->offsets_count = i;
    return MOBI_SUCCESS;
}

/**
 @brief Parser of INDX index entry
 
 @param[in,out] indx MOBIIndx structure, to be filled with parsed data
 @param[in] idxt MOBIIdxt structure with parsed IDXT index
 @param[in] tagx MOBITagx structure with parsed TAGX index
 @param[in,out] buf MOBIBuffer structure with index data
 @param[in] curr_number Sequential number of an index entry for current record
 @return MOBI_RET status code (on success MOBI_SUCCESS)
 */
static MOBI_RET mobi_parse_index_entry(MOBIIndx *indx, const MOBIIdxt idxt, const MOBITagx tagx, MOBIBuffer *buf, const size_t curr_number) {
    if (indx == NULL) {
        debug_print("%s", "INDX structure not initialized\n");
        return MOBI_INIT_FAILED;
    }
    const size_t entry_offset = indx->entries_count;
    const size_t entry_length = idxt.offsets[curr_number + 1] - idxt.offsets[curr_number];
    buf->offset = idxt.offsets[curr_number];
    size_t entry_number = curr_number + entry_offset;
    /* save original record maxlen */
    const size_t buf_maxlen = buf->maxlen;
    if (buf->offset + entry_length > buf_maxlen) {
        debug_print("Entry length too long: %zu\n", entry_length);
        return MOBI_DATA_CORRUPT;
    }
    buf->maxlen = buf->offset + entry_length;
    const size_t label_length = buffer_get8(buf);
    if (label_length > entry_length) {
        debug_print("Label length too long: %zu\n", label_length);
        return MOBI_DATA_CORRUPT;
    }
    indx->entries[entry_number].label = malloc(label_length + 1);
    buffer_getstring(indx->entries[entry_number].label, buf, label_length);
    debug_print("tag label[%zu]: %s\n", entry_number, indx->entries[entry_number].label);
    unsigned char *control_bytes;
    control_bytes = buf->data + buf->offset;
    buf->offset += tagx.control_byte_count;
    if (tagx.tags_count > 0) {
        typedef struct {
            uint8_t tag;
            uint8_t tag_value_count;
            uint32_t value_count;
            uint32_t value_bytes;
        } MOBIPtagx;
        MOBIPtagx ptagx[tagx.tags_count];
        uint32_t ptagx_count = 0;
        size_t len;
        indx->entries[entry_number].tags = malloc(tagx.tags_count * sizeof(MOBIIndexTag));
        size_t i = 0;
        while (i < tagx.tags_count) {
            if (tagx.tags[i].control_byte == 1) {
                control_bytes++;
                i++;
                continue;
            }
            uint32_t value = control_bytes[0] & tagx.tags[i].bitmask;
            if (value != 0) {
                /* FIXME: is it safe to use MOBI_NOTSET? */
                uint32_t value_count = MOBI_NOTSET;
                uint32_t value_bytes = MOBI_NOTSET;
                /* all bits of masked value are set */
                if (value == tagx.tags[i].bitmask) {
                    /* more than 1 bit set */
                    if (mobi_bitcount(tagx.tags[i].bitmask) > 1) {
                        /* read value bytes from entry */
                        len = 0;
                        value_bytes = buffer_get_varlen(buf, &len);
                    } else {
                        value_count = 1;
                    }
                } else {
                    uint8_t mask = tagx.tags[i].bitmask;
                    while ((mask & 1) == 0) {
                        mask >>= 1;
                        value >>= 1;
                    }
                    value_count = value;
                }
                ptagx[ptagx_count].tag = tagx.tags[i].tag;
                ptagx[ptagx_count].tag_value_count = tagx.tags[i].values_count;
                ptagx[ptagx_count].value_count = value_count;
                ptagx[ptagx_count].value_bytes = value_bytes;
                ptagx_count++;
            }
            i++;
        }
        indx->entries[entry_number].tags_count = ptagx_count;
        i = 0;
        while (i < ptagx_count) {
            uint32_t tagvalues_count = 0;
            /* FIXME: is it safe to use MOBI_NOTSET? */
            /* value count is set */
            if (ptagx[i].value_count != MOBI_NOTSET) {
                size_t count = ptagx[i].value_count * ptagx[i].tag_value_count;
                while (count-- && tagvalues_count < MOBI_INDX_MAXTAGVALUES) {
                    len = 0;
                    const uint32_t value_bytes = buffer_get_varlen(buf, &len);
                    indx->entries[entry_number].tags[i].tagvalues[tagvalues_count] = value_bytes;
                    tagvalues_count++;
                }
                /* value count is not set */
            } else {
                /* read value_bytes bytes */
                len = 0;
                while (len < ptagx[i].value_bytes && tagvalues_count < MOBI_INDX_MAXTAGVALUES) {
                    const uint32_t value_bytes = buffer_get_varlen(buf, &len);
                    indx->entries[entry_number].tags[i].tagvalues[tagvalues_count] = value_bytes;
                    tagvalues_count++;
                }
            }
            indx->entries[entry_number].tags[i].tagid = ptagx[i].tag;
            indx->entries[entry_number].tags[i].tagvalues_count = tagvalues_count;
            i++;
        }
    }
    /* restore buffer maxlen */
    buf->maxlen = buf_maxlen;
    return MOBI_SUCCESS;
}

/**
 @brief Parser of INDX record
 
 @param[in] indx_record MOBIPdbRecord structure with INDX record
 @param[in,out] indx MOBIIndx structure to be filled with parsed entries
 @param[in,out] tagx MOBITagx structure, will be filled with parsed TAGX section data if present in the INDX record,
                     otherwise TAGX data will be used to parse the record
 @return MOBI_RET status code (on success MOBI_SUCCESS)
 */
MOBI_RET mobi_parse_indx(const MOBIPdbRecord *indx_record, MOBIIndx *indx, MOBITagx *tagx) {
    MOBI_RET ret;
    MOBIBuffer *buf = buffer_init_null(indx_record->size);
    if (buf == NULL) {
        return MOBI_MALLOC_FAILED;
    }
    buf->data = indx_record->data;
    char indx_magic[5];
    buffer_getstring(indx_magic, buf, 4); /* 0: INDX magic */
    const uint32_t header_length = buffer_get32(buf); /* 4: header length */
    if (strncmp(indx_magic, INDX_MAGIC, 4) != 0 ||
        header_length == 0) {
        debug_print("INDX wrong magic: %s or header length: %u\n", indx_magic, header_length);
        buffer_free_null(buf);
        return MOBI_DATA_CORRUPT;
    }
    buf->offset += 4; /* 8: zeroes */
    /* FIXME: unused */
    indx->type = buffer_get32(buf); /* 12: 0 - normal, 2 - inflection */
    /* FIXME: unused */
    buf->offset += 4; /* 16: gen */
    const uint32_t idxt_offset = buffer_get32(buf); /* 20: IDXT offset */
    const size_t entries_count = buffer_get32(buf); /* 24: entries count */
    indx->encoding = buffer_get32(buf); /* 28: encoding */
    buf->offset += 4; /* 32: zeroes */
    const size_t total_entries_count = buffer_get32(buf); /* 36: total entries count */
    if (indx->total_entries_count == 0) {
        indx->total_entries_count = total_entries_count;
    }
    indx->ordt_offset = buffer_get32(buf); /* 40: ORDT offset */
    indx->ligt_offset = buffer_get32(buf); /* 44: LIGT offset */
    indx->ordt_entries_count = buffer_get32(buf); /* 48: ORDT entries count */
    indx->cncx_records_count = buffer_get32(buf); /* 52: CNCX entries count */
    //buf->offset += 124; /* 56: unknown */
    // 164: ocnt
    // 168: ORDT entries count
    // 172: ORDT1 offset
    // 176: ORDT2 offset
    // 180: otagx
    //uint32_t tagx_offset = buffer_get32(buf); /* 180: TAGX offset ? */
    /* FIXME: in dictionaries offset is moved by this value */
    //tagx_offset += buffer_get32(buf);
    /* buf->offset += 4; // 184: zeroes */
    buf->offset = header_length;
    
    /* TAGX metadata */
    /* if record contains TAGX section, read it and return */
    if (buffer_match_magic(buf, TAGX_MAGIC)) {
        ret = mobi_parse_tagx(buf, tagx);
        buffer_free_null(buf);
        indx->entries_count = entries_count;
        return ret;
    }
    /* IDXT entries offsets */
    if (idxt_offset == 0) {
        debug_print("%s", "Missing IDXT offset\n");
        buffer_free_null(buf);
        return MOBI_DATA_CORRUPT;
    }
    buf->offset = idxt_offset;
    MOBIIdxt idxt;
    uint32_t offsets[entries_count + 1];
    idxt.offsets = offsets;
    ret = mobi_parse_idxt(buf, &idxt, entries_count);
    if (ret != MOBI_SUCCESS) {
        debug_print("%s", "IDXT parsing failed\n");
        buffer_free_null(buf);
        return ret;
    }
    /* parse entries */
    if (entries_count > 0) {
        if (indx->entries == NULL) {
            indx->entries = malloc(indx->total_entries_count * sizeof(MOBIIndexEntry));
            if (indx->entries == NULL) {
                buffer_free_null(buf);
                return MOBI_MALLOC_FAILED;
            }
        }
        size_t i = 0;
        while (i < entries_count) {
            ret = mobi_parse_index_entry(indx, idxt, *tagx, buf, i++);
            if (ret != MOBI_SUCCESS) {
                buffer_free_null(buf);
                return ret;
            }
        }
        indx->entries_count += entries_count;

    }
    buffer_free_null(buf);
    return MOBI_SUCCESS;
}

/**
 @brief Parser of a set of index records
 
 @param[in] m MOBIData structure containing MOBI file metadata and data
 @param[in,out] indx MOBIIndx structure to be filled with parsed entries
 @param[in] indx_record_number Number of the first record of the set
 @return MOBI_RET status code (on success MOBI_SUCCESS)
 */
MOBI_RET mobi_parse_index(const MOBIData *m, MOBIIndx *indx, const size_t indx_record_number) {
    MOBI_RET ret;
    /* tagx.tags array will be allocated in mobi_parse_tagx */
    MOBITagx tagx = {.tags = NULL};
    /* parse first meta INDX record */
    MOBIPdbRecord *record = mobi_get_record_by_seqnumber(m, indx_record_number);
    ret = mobi_parse_indx(record, indx, &tagx);
    if (ret != MOBI_SUCCESS) {
        mobi_free_indx(indx);
        free(tagx.tags);
        indx = NULL;
        return ret;
    }
    size_t cncx_count = indx->cncx_records_count;
    /* parse remaining INDX records for the index */
    size_t count = indx->entries_count;
    indx->entries_count = 0;
    while (count--) {
        record = record->next;
        ret = mobi_parse_indx(record, indx, &tagx);
        if (ret != MOBI_SUCCESS) {
            mobi_free_indx(indx);
            free(tagx.tags);
            indx = NULL;
            return ret;
        }
    }
    /* copy pointer to first cncx record if present and set info from first record */
    if (cncx_count) {
        indx->cncx_records_count = cncx_count;
        indx->cncx_record = record->next;
    }
    free(tagx.tags);
    return MOBI_SUCCESS;
}

/**
 @brief Get a value of tag[tagid][tagindex] for given index entry
 
 @param[in,out] tagvalue Will be set to a tag value
 @param[in] entry Index entry to be search for the value
 @param[in] tag_arr Array: tag_arr[0] = tagid, tag_arr[1] = tagindex
 @return MOBI_RET status code (on success MOBI_SUCCESS)
 */
MOBI_RET mobi_get_indxentry_tagvalue(uint32_t *tagvalue, const MOBIIndexEntry *entry, const unsigned tag_arr[]) {
    if (entry == NULL) {
        debug_print("%s", "INDX entry not initialized\n");
        return MOBI_INIT_FAILED;
    }
    size_t i = 0;
    while (i < entry->tags_count) {
        if (entry->tags[i].tagid == tag_arr[0]) {
            *tagvalue = entry->tags[i].tagvalues[tag_arr[1]];
            return MOBI_SUCCESS;
        }
        i++;
    }
    debug_print("tag[%i][%i] not found in entry: %s\n", tag_arr[0], tag_arr[1], entry->label)
    ;
    return MOBI_DATA_CORRUPT;
}


/**
 @brief Get compiled index entry string

 Allocates memory for the string. Must be freed by caller.
 
 @param[in] cncx_record MOBIPdbRecord structure with cncx record
 @param[in] cncx_offset Offset of string entry from the beginning of the record
 @return Entry string
 */
char * mobi_get_cncx_string(const MOBIPdbRecord *cncx_record, const uint32_t cncx_offset) {
    /* TODO: handle multiple cncx records */
    MOBIBuffer *buf = buffer_init_null(cncx_record->size);
    buf->data = cncx_record->data;
    buf->offset = cncx_offset;
    size_t len = 0;
    const uint32_t string_length = buffer_get_varlen(buf, &len);
    char *string = malloc(string_length + 1);
    if (string) {
        buffer_getstring(string, buf, string_length);
        buffer_free_null(buf);
    }
    return string;
}
