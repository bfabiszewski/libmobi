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

#define _GNU_SOURCE 1
#ifndef __USE_BSD
#define __USE_BSD /* for strdup on linux/glibc */
#endif

#include <string.h>
#include <stdlib.h>
#include <stdint.h>

#include "index.h"
#include "util.h"
#include "memory.h"
#include "debug.h"
#include "buffer.h"

size_t mobi_indx_get_label(unsigned char *output, MOBIBuffer *buf, const size_t length, const MOBIEncoding encoding) {
    if (!output) {
        buf->error = MOBI_PARAM_ERR;
        return 0;
    }
    if (buf->offset + length > buf->maxlen) {
        debug_print("%s", "End of buffer\n");
        buf->error = MOBI_BUFFER_END;
        return 0;
    }
    size_t output_length = 0;
    size_t i = 0;
    while (i < length && output_length < INDX_LABEL_SIZEMAX) {
        unsigned char c = buffer_get8(buf);
        i++;
        if (c <= 5) {
            unsigned char c2 = buffer_get8(buf);
            if (c2 <= 5) {
                buffer_seek(buf, -1);
                continue;
            }
            // FIXME: is it needed for newer mobi files?
            uint16_t liga = mobi_decode_ligature(c, c2, encoding);
            if (liga & 0xff00) {
                /* first byte */
                *output++ = liga >> 8;
                output_length++;
            }
            /* second byte */
            c = (uint8_t) liga;
            i++;
        }
        *output++ = c;
        output_length++;
    }
    *output = '\0';
    return output_length;
}

/**
 @brief Parser of ORDT section of INDX record
 
 @param[in,out] buf MOBIBuffer structure, offset pointing at beginning of TAGX section
 @param[in,out] ordt MOBIOrdt structure to be filled by the function
 @return MOBI_RET status code (on success MOBI_SUCCESS)
 */
static MOBI_RET mobi_parse_ordt(MOBIBuffer *buf, MOBIOrdt *ordt) {
    /* read ORDT1 */
    if (buffer_match_magic(buf, ORDT_MAGIC)) {
        debug_print("%s\n", "ORDT1 section found");
        buffer_seek(buf, 4);
        ordt->ordt1 = malloc(ordt->offsets_count * sizeof(*ordt->ordt1));
        if (ordt->ordt1 == NULL) {
            debug_print("%s", "Memory allocation failed for ORDT1 offsets\n");
            return MOBI_MALLOC_FAILED;
        }
        size_t i = 0;
        while (i < ordt->offsets_count) {
            ordt->ordt1[i++] = buffer_get8(buf);
        }
        debug_print("ORDT1: read %zu entries\n", ordt->offsets_count);
    }
    /* read ORDT2 */
    buffer_setpos(buf, ordt->ordt2_pos);
    if (buffer_match_magic(buf, ORDT_MAGIC)) {
        debug_print("%s\n", "ORDT2 section found");
        buffer_seek(buf, 4);
        ordt->ordt2 = malloc(ordt->offsets_count * sizeof(*ordt->ordt2));
        if (ordt->ordt2 == NULL) {
            debug_print("%s", "Memory allocation failed for ORDT2 offsets\n");
            return MOBI_MALLOC_FAILED;
        }
        size_t i = 0;
        while (i < ordt->offsets_count) {
            ordt->ordt2[i++] = buffer_get16(buf);
        }
        debug_print("ORDT2: read %zu entries\n", ordt->offsets_count);
    }
    return MOBI_SUCCESS;
}

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
    buffer_seek(buf, 4); /* skip header */
    const uint32_t tagx_header_length = buffer_get32(buf);
    if (tagx_header_length < 12) {
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
 @brief Get encoded character from dictionary index
 The characters are offsets into ORDT table
 
 @param[in] ordt MOBIOrdt structure (ORDT data and metadata)
 @param[in,out] buf MOBIBuffer structure with index data
 @param[in,out] offset Value read from buffer
 @return Number of bytes read (zero in case of error)
 */
size_t mobi_ordt_getbuffer(const MOBIOrdt *ordt, MOBIBuffer *buf, uint16_t *offset) {
    size_t i = 0;
    if (ordt->type == 1) {
        *offset = buffer_get8(buf);
        i++;
    } else {
        *offset = buffer_get16(buf);
        i += 2;
    }
    return i;
}

/**
 @brief Fetch UTF-16 value from ORDT2 table
 
 @param[in] ordt MOBIOrdt structure (ORDT data and metadata)
 @param[in] offset Offset in ORDT2 table
 @return UTF-16 code point
 */
uint16_t mobi_ordt_lookup(const MOBIOrdt *ordt, const uint16_t offset) {
    uint16_t utf16;
    if (offset < ordt->offsets_count) {
        utf16 = ordt->ordt2[offset];
    } else {
        utf16 = offset;
    }
    return utf16;
}

/**
 @brief Get UTF-8 string from buffer, decoded by lookups in ORDT2 table
 
 @param[in] ordt MOBIOrdt structure (ORDT data and metadata)
 @param[in,out] buf MOBIBuffer structure with input string
 @param[in,out] output Output buffer (INDX_LABEL_SIZEMAX bytes)
 @param[in] length Length of input string contained in buf
 @return Number of bytes read
 */
size_t mobi_getstring_ordt(const MOBIOrdt *ordt, MOBIBuffer *buf, unsigned char *output, size_t length) {
    size_t i = 0;
    size_t output_length = 0;
    const uint32_t bytemask = 0xbf;
    const uint32_t bytemark = 0x80;
    const uint32_t uni_replacement = 0xfffd;
    const uint32_t surrogate_offset = 0x35fdc00;
    static const uint8_t init_byte[7] = { 0x00, 0x00, 0xc0, 0xe0, 0xf0, 0xf8, 0xfc };
    while (i < length) {
        uint16_t offset;
        i += mobi_ordt_getbuffer(ordt, buf, &offset);
        uint32_t codepoint = mobi_ordt_lookup(ordt, offset);
        /* convert UTF-16 surrogates into UTF-32 */
        if (codepoint >= 0xd800 && codepoint <= 0xdbff) {
            size_t k = mobi_ordt_getbuffer(ordt, buf, &offset);
            uint32_t codepoint2 = mobi_ordt_lookup(ordt, offset);
            if (codepoint2 >= 0xdc00 && codepoint2 <= 0xdfff) {
                i += k;
                codepoint = (codepoint << 10) + codepoint2 - surrogate_offset;
            } else {
                /* illegal unpaired high surrogate */
                /* rewind buffer to codepoint2 */
                buffer_seek(buf, (int) -k);
                codepoint = uni_replacement;
                debug_print("Invalid code point: %u\n", codepoint);
            }
        }
        if ((codepoint >= 0xdc00 && codepoint <= 0xdfff) /* unpaired low surrogate */
            || (codepoint >= 0xfdd0 && codepoint <= 0xfdef) /* invalid characters */
            || (codepoint & 0xfffe) == 0xfffe /* reserved characters */
            || codepoint == 0 /* remove zeroes */) {
            codepoint = uni_replacement;
            debug_print("Invalid code point: %u\n", codepoint);
        }
        /* Conversion routine based on unicode's ConvertUTF.c */
        size_t bytes;
        if (codepoint < 0x80) { bytes = 1; }
        else if (codepoint < 0x800) { bytes = 2; }
        else if (codepoint < 0x10000) { bytes = 3; }
        else if (codepoint < 0x110000) { bytes = 4; }
        else {
            bytes = 3;
            codepoint = uni_replacement;
            debug_print("Invalid code point: %u\n", codepoint);
        }
        if (output_length + bytes >= INDX_LABEL_SIZEMAX) {
            debug_print("%s\n", "INDX label too long");
            break;
        }
        output += bytes;
        switch (bytes) { /* note: everything falls through. */
            case 4: *--output = (uint8_t)((codepoint | bytemark) & bytemask); codepoint >>= 6;
            case 3: *--output = (uint8_t)((codepoint | bytemark) & bytemask); codepoint >>= 6;
            case 2: *--output = (uint8_t)((codepoint | bytemark) & bytemask); codepoint >>= 6;
            case 1: *--output = (uint8_t)(codepoint | init_byte[bytes]);
        }
        output += bytes;
        output_length += bytes;
    }
    *output = '\0';
    return output_length;
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
static MOBI_RET mobi_parse_index_entry(MOBIIndx *indx, const MOBIIdxt idxt, const MOBITagx *tagx, const MOBIOrdt *ordt, MOBIBuffer *buf, const size_t curr_number) {
    if (indx == NULL) {
        debug_print("%s", "INDX structure not initialized\n");
        return MOBI_INIT_FAILED;
    }
    const size_t entry_offset = indx->entries_count;
    const size_t entry_length = idxt.offsets[curr_number + 1] - idxt.offsets[curr_number];
    buffer_setpos(buf, idxt.offsets[curr_number]);
    size_t entry_number = curr_number + entry_offset;
    /* save original record maxlen */
    const size_t buf_maxlen = buf->maxlen;
    if (buf->offset + entry_length > buf_maxlen) {
        debug_print("Entry length too long: %zu\n", entry_length);
        return MOBI_DATA_CORRUPT;
    }
    buf->maxlen = buf->offset + entry_length;
    size_t label_length = buffer_get8(buf);
    if (label_length > entry_length) {
        debug_print("Label length too long: %zu\n", label_length);
        return MOBI_DATA_CORRUPT;
    }
    char text[INDX_LABEL_SIZEMAX];
    /* FIXME: what is ORDT1 for? */
    if (ordt->ordt2) {
        label_length = mobi_getstring_ordt(ordt, buf, (unsigned char*) text, label_length);
    } else {
        label_length = mobi_indx_get_label((unsigned char*) text, buf, label_length, indx->encoding);
    }
    indx->entries[entry_number].label = malloc(label_length + 1);
    if (indx->entries[entry_number].label == NULL) {
        debug_print("Memory allocation failed (%zu bytes)\n", label_length);
        return MOBI_MALLOC_FAILED;
    }
    strncpy(indx->entries[entry_number].label, text, label_length + 1);
    //debug_print("tag label[%zu]: %s\n", entry_number, indx->entries[entry_number].label);
    unsigned char *control_bytes;
    control_bytes = buf->data + buf->offset;
    buffer_seek(buf, (int) tagx->control_byte_count);
    if (tagx->tags_count > 0) {
        typedef struct {
            uint8_t tag;
            uint8_t tag_value_count;
            uint32_t value_count;
            uint32_t value_bytes;
        } MOBIPtagx;
        MOBIPtagx ptagx[tagx->tags_count];
        uint32_t ptagx_count = 0;
        size_t len;
        size_t i = 0;
        while (i < tagx->tags_count) {
            if (tagx->tags[i].control_byte == 1) {
                control_bytes++;
                i++;
                continue;
            }
            uint32_t value = control_bytes[0] & tagx->tags[i].bitmask;
            if (value != 0) {
                /* FIXME: is it safe to use MOBI_NOTSET? */
                uint32_t value_count = MOBI_NOTSET;
                uint32_t value_bytes = MOBI_NOTSET;
                /* all bits of masked value are set */
                if (value == tagx->tags[i].bitmask) {
                    /* more than 1 bit set */
                    if (mobi_bitcount(tagx->tags[i].bitmask) > 1) {
                        /* read value bytes from entry */
                        len = 0;
                        value_bytes = buffer_get_varlen(buf, &len);
                    } else {
                        value_count = 1;
                    }
                } else {
                    uint8_t mask = tagx->tags[i].bitmask;
                    while ((mask & 1) == 0) {
                        mask >>= 1;
                        value >>= 1;
                    }
                    value_count = value;
                }
                ptagx[ptagx_count].tag = tagx->tags[i].tag;
                ptagx[ptagx_count].tag_value_count = tagx->tags[i].values_count;
                ptagx[ptagx_count].value_count = value_count;
                ptagx[ptagx_count].value_bytes = value_bytes;
                ptagx_count++;
            }
            i++;
        }
        indx->entries[entry_number].tags = malloc(tagx->tags_count * sizeof(MOBIIndexTag));
        indx->entries[entry_number].tags_count = 0;
        i = 0;
        while (i < ptagx_count) {
            uint32_t tagvalues_count = 0;
            /* FIXME: is it safe to use MOBI_NOTSET? */
            /* value count is set */
            uint32_t tagvalues[INDX_TAGVALUES_MAX];
            if (ptagx[i].value_count != MOBI_NOTSET) {
                size_t count = ptagx[i].value_count * ptagx[i].tag_value_count;
                while (count-- && tagvalues_count < INDX_TAGVALUES_MAX) {
                    len = 0;
                    const uint32_t value_bytes = buffer_get_varlen(buf, &len);
                    tagvalues[tagvalues_count++] = value_bytes;
                }
            /* value count is not set */
            } else {
                /* read value_bytes bytes */
                len = 0;
                while (len < ptagx[i].value_bytes && tagvalues_count < INDX_TAGVALUES_MAX) {
                    const uint32_t value_bytes = buffer_get_varlen(buf, &len);
                    tagvalues[tagvalues_count++] = value_bytes;
                }
            }
            if (tagvalues_count) {
                const size_t arr_size = tagvalues_count * sizeof(*indx->entries[entry_number].tags[i].tagvalues);
                indx->entries[entry_number].tags[i].tagvalues = malloc(arr_size);
                if (indx->entries[entry_number].tags[i].tagvalues == NULL) {
                    debug_print("Memory allocation failed (%zu bytes)\n", arr_size);
                    return MOBI_MALLOC_FAILED;
                }
                memcpy(indx->entries[entry_number].tags[i].tagvalues, tagvalues, arr_size);
            } else {
                indx->entries[entry_number].tags[i].tagvalues = NULL;
            }
            indx->entries[entry_number].tags[i].tagid = ptagx[i].tag;
            indx->entries[entry_number].tags[i].tagvalues_count = tagvalues_count;
            indx->entries[entry_number].tags_count++;
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
 @param[in,out] tagx MOBITagx structure, will be filled with parsed TAGX section data if present in the INDX record, otherwise TAGX data will be used to parse the record
 @param[in,out] ordt MOBIOrdt structure, will be filled with parsed ORDT sections
 @return MOBI_RET status code (on success MOBI_SUCCESS)
 */
MOBI_RET mobi_parse_indx(const MOBIPdbRecord *indx_record, MOBIIndx *indx, MOBITagx *tagx, MOBIOrdt *ordt) {
    MOBI_RET ret;
    MOBIBuffer *buf = buffer_init_null(indx_record->size);
    if (buf == NULL) {
        debug_print("%s\n", "Memory allocation failed");
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
    buffer_seek(buf, 4); /* 8: zeroes */
    /* FIXME: unused */
    indx->type = buffer_get32(buf); /* 12: 0 - normal, 2 - inflection */
    /* FIXME: unused */
    buffer_seek(buf, 4); /* 16: gen */
    const uint32_t idxt_offset = buffer_get32(buf); /* 20: IDXT offset */
    const size_t entries_count = buffer_get32(buf); /* 24: entries count */
    const uint32_t encoding = buffer_get32(buf); /* 28: encoding */
    buffer_seek(buf, 4); /* 32: zeroes */
    const size_t total_entries_count = buffer_get32(buf); /* 36: total entries count */
    if (indx->total_entries_count == 0) {
        indx->total_entries_count = total_entries_count;
    }
    indx->ordt_offset = buffer_get32(buf); /* 40: ORDT offset */
    indx->ligt_offset = buffer_get32(buf); /* 44: LIGT offset */
    indx->ordt_entries_count = buffer_get32(buf); /* 48: ORDT entries count */
    indx->cncx_records_count = buffer_get32(buf); /* 52: CNCX entries count */
    buffer_setpos(buf, 164);
    uint32_t ordt_type = buffer_get32(buf); /* 164: ORDT type */
    uint32_t ordt_entries_count = buffer_get32(buf); /* 168: ORDT entries count */
    uint32_t ordt1_offset = buffer_get32(buf); /* 172: ORDT1 offset */
    uint32_t ordt2_offset = buffer_get32(buf); /* 176: ORDT2 offset */
    uint32_t index_name_offset = buffer_get32(buf); /* 180: Default index string offset ? */
    uint32_t index_name_length = buffer_get32(buf); /* 184: Default index string length ? */
    
    buffer_setpos(buf, header_length);
    
    /* TAGX metadata */
    /* if record contains TAGX section, read it (and ORDT) and return */
    if (buffer_match_magic(buf, TAGX_MAGIC)) {
        indx->encoding = encoding;
        ret = mobi_parse_tagx(buf, tagx);
        if (ret != MOBI_SUCCESS) {
            buffer_free_null(buf);
            return ret;
        }
        if (indx->encoding == MOBI_UTF16 || ordt_entries_count > 0) {
            /* parse ORDT sections */
            buffer_setpos(buf, ordt1_offset);
            ordt->offsets_count = ordt_entries_count;
            ordt->type = ordt_type;
            ordt->ordt1_pos = ordt1_offset;
            ordt->ordt2_pos = ordt2_offset;
            ret = mobi_parse_ordt(buf, ordt);
            debug_print("ORDT: %u, %u, %u, %u\n", ordt_type, ordt_entries_count, ordt1_offset, ordt2_offset);
        }
        if (index_name_offset > 0 && index_name_length > 0) {
            if (index_name_length <= header_length - index_name_offset) {
                buffer_setpos(buf, index_name_offset);
                char *name = malloc(index_name_length + 1);
                buffer_getstring(name, buf, index_name_length);
                indx->orth_index_name = name;
                debug_print("Orth index name: %s\n", name);
            }
        }
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
    buffer_setpos(buf, idxt_offset);
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
                debug_print("%s\n", "Memory allocation failed");
                return MOBI_MALLOC_FAILED;
            }
        }
        size_t i = 0;
        while (i < entries_count) {
            ret = mobi_parse_index_entry(indx, idxt, tagx, ordt, buf, i++);
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
    /* tagx->tags array will be allocated in mobi_parse_tagx */
    MOBITagx *tagx = calloc(1, sizeof(MOBITagx));
    if (tagx == NULL) {
        mobi_free_indx(indx);
        debug_print("%s\n", "Memory allocation failed");
        return MOBI_MALLOC_FAILED;
    }
    /* ordt->ordt1 and ordt.ordt2 arrays will be allocated in mobi_parse_ordt */
    MOBIOrdt *ordt = calloc(1, sizeof(MOBIOrdt));
    if (ordt == NULL) {
        mobi_free_indx(indx);
        mobi_free_tagx(tagx);
        debug_print("%s\n", "Memory allocation failed");
        return MOBI_MALLOC_FAILED;
    }
    /* parse first meta INDX record */
    MOBIPdbRecord *record = mobi_get_record_by_seqnumber(m, indx_record_number);
    ret = mobi_parse_indx(record, indx, tagx, ordt);
    if (ret != MOBI_SUCCESS) {
        mobi_free_indx(indx);
        mobi_free_tagx(tagx);
        mobi_free_ordt(ordt);
        return ret;
    }
    size_t cncx_count = indx->cncx_records_count;
    /* parse remaining INDX records for the index */
    size_t count = indx->entries_count;
    indx->entries_count = 0;
    while (count--) {
        record = record->next;
        ret = mobi_parse_indx(record, indx, tagx, ordt);
        if (ret != MOBI_SUCCESS) {
            mobi_free_indx(indx);
            mobi_free_tagx(tagx);
            mobi_free_ordt(ordt);
            return ret;
        }
    }
    /* copy pointer to first cncx record if present and set info from first record */
    if (cncx_count) {
        indx->cncx_records_count = cncx_count;
        indx->cncx_record = record->next;
    }
    mobi_free_tagx(tagx);
    mobi_free_ordt(ordt);
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
            if (entry->tags[i].tagvalues_count > tag_arr[1]) {
                *tagvalue = entry->tags[i].tagvalues[tag_arr[1]];
                return MOBI_SUCCESS;
            }
            break;
        }
        i++;
    }
    //debug_print("tag[%i][%i] not found in entry: %s\n", tag_arr[0], tag_arr[1], entry->label);
    return MOBI_DATA_CORRUPT;
}

/**
 @brief Get array of tagvalues of tag[tagid] for given index entry
 
 @param[in,out] tagarr Pointer to tagvalues array
 @param[in] entry Index entry to be search for the value
 @param[in] tagid Id of the tag
 @return Size of the array (zero on failure)
 */
size_t mobi_get_indxentry_tagarray(uint32_t **tagarr, const MOBIIndexEntry *entry, const size_t tagid) {
    if (entry == NULL) {
        debug_print("%s", "INDX entry not initialized\n");
        return 0;
    }
    size_t i = 0;
    while (i < entry->tags_count) {
        if (entry->tags[i].tagid == tagid) {
            *tagarr = entry->tags[i].tagvalues;
            return entry->tags[i].tagvalues_count;
        }
        i++;
    }
    debug_print("tag[%zu] not found in entry: %s\n", tagid, entry->label);
    return 0;
}

/**
 @brief Check if given tagid is present in the index
 
 @param[in] indx Index MOBIIndx structure
 @param[in] tagid Id of the tag
 @return True on success, false otherwise
 */
bool mobi_indx_has_tag(const MOBIIndx *indx, const size_t tagid) {
    if (indx) {
        for (size_t i = 0; i < indx->entries_count; i++) {
            MOBIIndexEntry entry = indx->entries[i];
            for(size_t j = 0; j < entry.tags_count; j++) {
                if (entry.tags[j].tagid == tagid) {
                    return true;
                }
            }
        }
    }
    return false;
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
    buffer_setpos(buf, cncx_offset);
    size_t len = 0;
    const uint32_t string_length = buffer_get_varlen(buf, &len);
    char *string = malloc(string_length + 1);
    if (string) {
        buffer_getstring(string, buf, string_length);
        buffer_free_null(buf);
    }
    return string;
}

/**
 @brief Get flat index entry string
 
 Allocates memory for the string. Must be freed by caller.
 
 @param[in] cncx_record MOBIPdbRecord structure with cncx record
 @param[in] cncx_offset Offset of string entry from the beginning of the record
 @param[in] length Length of the string to be extracted
 @return Entry string
 */
char * mobi_get_cncx_string_flat(const MOBIPdbRecord *cncx_record, const uint32_t cncx_offset, const size_t length) {
    /* TODO: handle multiple cncx records */
    MOBIBuffer *buf = buffer_init_null(cncx_record->size);
    buf->data = cncx_record->data;
    buffer_setpos(buf, cncx_offset);
    char *string = malloc(length + 1);
    if (string) {
        buffer_getstring(string, buf, length);
        buffer_free_null(buf);
    }
    return string;
}

/**
 @brief Decode compiled infl index entry
 
 Buffer decoded must be initialized with basic index entry.
 Basic index entry will be transformed into inflected form,
 based on compiled rule.
 Min. size of input buffer (decoded) must be INDX_INFLBUF_SIZEMAX + 1
 
 @param[in,out] decoded Decoded entry string
 @param[in,out] decoded_size Decoded entry size
 @param[in] rule Compiled rule
 @return MOBI_RET status code (on success MOBI_SUCCESS)
 */
MOBI_RET mobi_decode_infl(unsigned char *decoded, int *decoded_size, const unsigned char *rule) {
    int pos = *decoded_size;
    char mod = 'i';
    char dir = '<', olddir;
    unsigned char c;
    while ((c = *rule++)) {
        if (c <= 4) {
            mod = (c <= 2) ? 'i' : 'd'; /* insert, delete */
            olddir = dir;
            dir = (c & 2) ? '<' : '>'; /* left, right */
            if (olddir != dir && olddir) {
                pos = (c & 2) ? *decoded_size : 0;
            }
        }
        else if (c > 10 && c < 20) {
            if (dir == '>') {
                pos = *decoded_size;
            }
            pos -= c - 10;
            dir = 0;
            if (pos < 0 || pos > *decoded_size) {
                debug_print("Position setting failed (%s)\n", decoded);
                return MOBI_DATA_CORRUPT;
            }
        }
        else {
            if (mod == 'i') {
                const unsigned char *s = decoded + pos;
                unsigned char *d = decoded + pos + 1;
                const int l = *decoded_size - pos;
                if (l < 0 || d + l > decoded + INDX_INFLBUF_SIZEMAX) {
                    debug_print("Out of buffer in %s at pos: %i\n", decoded, pos);
                    return MOBI_DATA_CORRUPT;
                }
                memmove(d, s, l);
                decoded[pos] = c;
                (*decoded_size)++;
                if (dir == '>') { pos++; }
            } else {
                if (dir == '<') { pos--; }
                const unsigned char *s = decoded + pos + 1;
                unsigned char *d = decoded + pos;
                const int l = *decoded_size - pos;
                if (l < 0 || d + l > decoded + INDX_INFLBUF_SIZEMAX) {
                    debug_print("Out of buffer in %s at pos: %i\n", decoded, pos);
                    return MOBI_DATA_CORRUPT;
                }
                if (decoded[pos] != c) {
                    debug_print("Character mismatch in %s at pos: %i (%c != %c)\n", decoded, pos, decoded[pos], c);
                    return MOBI_DATA_CORRUPT;
                }
                memmove(d, s, l);
                (*decoded_size)--;
            }
        }
    }
    return MOBI_SUCCESS;
}

size_t mobi_trie_get_inflgroups(char **infl_strings, MOBITrie *root, const char *string) {
    /* travers trie and get values for each substring */
    if (root == NULL) {
        return MOBI_PARAM_ERR;
    }
    size_t count = 0;
    size_t length = strlen(string);
    MOBITrie *node = root;
    while (node && length > 0) {
        char **values = NULL;
        size_t values_count = 0;
        node = mobi_trie_get_next(&values, &values_count, node, string[length - 1]);
        length--;
        for (size_t j = 0; j < values_count; j++) {
            if (count == INDX_INFLSTRINGS_MAX) {
                debug_print("Inflection strings array too small (%d)\n", INDX_INFLSTRINGS_MAX);
                break;
            }
            char infl_string[INDX_LABEL_SIZEMAX + 1];
            const size_t suffix_length = strlen(values[j]);
            if (length + suffix_length > INDX_LABEL_SIZEMAX) {
                debug_print("Label too long (%zu + %zu)\n", length, suffix_length);
                continue;
            }
            memcpy(infl_string, string, length);
            memcpy(infl_string + length, values[j], suffix_length);
            infl_string[length + suffix_length] = '\0';
            infl_strings[count++] = strdup(infl_string);
        }
    }
    return count;
}

MOBI_RET mobi_trie_insert_infl(MOBITrie **root, const MOBIIndx *indx, size_t i) {
    MOBIIndexEntry e = indx->entries[i];
    char *inflected = e.label;
    for (size_t j = 0; j < e.tags_count; j++) {
        MOBIIndexTag t = e.tags[j];
        if (t.tagid == INDX_TAGARR_INFL_PARTS_V1) {
            for (size_t k = 0; k < t.tagvalues_count - 1; k += 2) {
                uint32_t len = t.tagvalues[k];
                uint32_t offset = t.tagvalues[k + 1];
                char *base = mobi_get_cncx_string_flat(indx->cncx_record, offset, len);
                if (base == NULL) {
                    return MOBI_MALLOC_FAILED;
                }
                MOBI_RET ret = mobi_trie_insert_reversed(root, base, inflected);
                free(base);
                if (ret != MOBI_SUCCESS) {
                    return ret;
                }
            }
        }
    }
    return MOBI_SUCCESS;
}
