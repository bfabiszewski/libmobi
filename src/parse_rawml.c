/** @file parse_rawml.c
 *  @brief Functions for parsing rawml markup
 *
 * Copyright (c) 2014 Bartek Fabiszewski
 * http://www.fabiszewski.net
 *
 * This file is part of libmobi.
 * Licensed under LGPL, either version 3, or any later.
 * See <http://www.gnu.org/licenses/>
 */

#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "parse_rawml.h"
#include "util.h"
#include "opf.h"
#include "index.h"
#include "debug.h"


/**
 @brief Convert kindle:pos:fid:x:off:y to offset in rawml raw text file
 
 @param[in] rawml MOBIRawml parsed records structure
 @param[in] pos_fid X value of pos:fid:x
 @param[in] pos_off Y value of off:y
 @return Offset in rawml buffer on success, SIZE_MAX otherwise
 */
size_t mobi_get_rawlink_location(const MOBIRawml *rawml, const uint32_t pos_fid, const uint32_t pos_off) {
    if (!rawml || !rawml->frag || !rawml->frag->entries ) {
        debug_print("%s", "Initialization failed\n");
        return SIZE_MAX;
    }
    if (pos_fid >= rawml->frag->entries_count) {
        debug_print("%s", "pos_fid not found\n");
        return SIZE_MAX;
    }
    const MOBIIndexEntry *entry = &rawml->frag->entries[pos_fid];
    const size_t insert_position = strtoul(entry->label, NULL, 10);
    size_t file_offset = insert_position + pos_off;
    return file_offset;
}

/**
 @brief Find first occurence of attribute to be replaced in KF7 html
 
 It searches for filepos and recindex attributes
 
 @param[in,out] result MOBIResult structure will be filled with found data
 @param[in] data_start Beginning of the memory area to search in
 @param[in] data_end End of the memory area to search in
 @return MOBI_RET status code (on success MOBI_SUCCESS)
 */
MOBI_RET mobi_search_links_kf7(MOBIResult *result, const unsigned char *data_start, const unsigned char *data_end) {
    if (!result) {
        debug_print("Result structure is null%s", "\n");
        return MOBI_PARAM_ERR;
    }
    result->start = result->end = NULL;
    *(result->value) = '\0';
    if (!data_start || !data_end) {
        debug_print("Data is null%s", "\n");
        return MOBI_PARAM_ERR;
    }
    const char *needle1 = "filepos=";
    const char *needle2 = "recindex=";
    const size_t needle1_length = strlen(needle1);
    const size_t needle2_length = strlen(needle2);
    const size_t needle_length = max(needle1_length,needle2_length);
    if (data_start + needle_length > data_end) {
        return MOBI_SUCCESS;
    }
    unsigned char *data = (unsigned char *) data_start;
    unsigned char last_border = '>';
    const unsigned char tag_open = '<';
    const unsigned char tag_close = '>';
    while (data <= data_end) {
        if (*data == tag_open || *data == tag_close) {
            last_border = *data;
        }
        if (data + needle_length <= data_end &&
            (memcmp(data, needle1, needle1_length) == 0 ||
             memcmp(data, needle2, needle2_length) == 0)) {
                /* found match */
                if (last_border != tag_open) {
                    /* opening char not found, not an attribute */
                    data += needle_length;
                    continue;
                }
                /* go to attribute  beginning */
                while (data >= data_start && !isspace(*data) && *data != tag_open) {
                    data--;
                }
                result->start = ++data;
                /* now go forward */
                int i = 0;
                while (data <= data_end && !isspace(*data) && *data != tag_close && i < MOBI_ATTRVALUE_MAXSIZE) {
                    result->value[i++] = (char) *data++;
                }
                /* self closing tag '/>' */
                if (*(data - 1) == '/' && *data == '>') {
                    --data; --i;
                }
                result->end = data;
                result->value[i] = '\0';
                return MOBI_SUCCESS;
            }
        data++;
    }
    return MOBI_SUCCESS;
}

/**
 @brief Find first occurence of markup attribute with given string
 
 @param[in,out] result MOBIResult structure will be filled with found data
 @param[in] data_start Beginning of the memory area to search in
 @param[in] data_end End of the memory area to search in
 @param[in] type Type of data (T_HTML or T_CSS)
 @param[in] needle String to find
 @return MOBI_RET status code (on success MOBI_SUCCESS)
 */
MOBI_RET mobi_search_markup(MOBIResult *result, const unsigned char *data_start, const unsigned char *data_end, const MOBIFiletype type, const char *needle) {
        if (!result) {
            debug_print("Result structure is null%s", "\n");
            return MOBI_PARAM_ERR;
        }
        result->start = result->end = NULL;
        *(result->value) = '\0';
        if (!data_start || !data_end) {
            debug_print("Data is null%s", "\n");
            return MOBI_PARAM_ERR;
        }
        size_t needle_length = strlen(needle);
        if (needle_length > MOBI_ATTRNAME_MAXSIZE) {
            debug_print("Attribute too long: %zu\n", needle_length);
            return MOBI_PARAM_ERR;
        }
        if (data_start + needle_length > data_end) {
            return MOBI_SUCCESS;
        }
        unsigned char *data = (unsigned char *) data_start;
        unsigned char last_border = '>';
        unsigned char tag_open;
        unsigned char tag_close;
        if (type == T_CSS) {
            tag_open = '{';
            tag_close = '}';
        } else {
            tag_open = '<';
            tag_close = '>';
        }
        while (data <= data_end) {
            if (*data == tag_open || *data == tag_close) {
                last_border = *data;
            }
            if (data + needle_length <= data_end && memcmp(data, needle, needle_length) == 0) {
                /* found match */
                if (last_border != tag_open) {
                    /* opening char not found, not an attribute */
                    data += needle_length;
                    continue;
                }
                /* go to attribute value beginning */
                while (data >= data_start && !isspace(*data) && *data != tag_open && *data != '=' && *data != '(') {
                    data--;
                }
                result->is_url = (*data == '(');
                result->start = ++data;
                /* now go forward */
                int i = 0;
                while (data <= data_end && !isspace(*data) && *data != tag_close && *data != ')' && i < MOBI_ATTRVALUE_MAXSIZE) {
                    result->value[i++] = (char) *data++;
                }
                /* self closing tag '/>' */
                if (*(data - 1) == '/' && *data == '>') {
                    --data; --i;
                }
                result->end = data;
                result->value[i] = '\0';
                return MOBI_SUCCESS;
            }
            data++;
        }
        return MOBI_SUCCESS;
}

/**
 @brief Find first occurence of attribute part to be replaced in KF8 html/css
 
 It searches for "kindle:" value in attributes
 
 @param[in,out] result MOBIResult structure will be filled with found data
 @param[in] data_start Beginning of the memory area to search in
 @param[in] data_end End of the memory area to search in
 @param[in] type Type of data (T_HTML or T_CSS)
 @return MOBI_RET status code (on success MOBI_SUCCESS)
 */
MOBI_RET mobi_search_links_kf8(MOBIResult *result, const unsigned char *data_start, const unsigned char *data_end, const MOBIFiletype type) {
    return mobi_search_markup(result, data_start, data_end, type, "kindle:");
}

/**
 @brief Get value and offset of the first found attribute with given name
 
 @param[in,out] value String value of the attribute, will be filled by the function, zero length if not found
 @param[in] data Data to search in
 @param[in] size Data size
 @param[in] attribute Attribute name
 @param[in] only_quoted Require the value to be quoted if true, allow no quotes (eg. filepos=00001) if false
 @return Offset from the beginning of the data, SIZE_MAX if not found
 */
size_t mobi_get_attribute_value(char *value, const unsigned char *data, const size_t size, const char *attribute, bool only_quoted) {
    if (!data) {
        debug_print("Data is null%s", "\n");
        return SIZE_MAX;
    }
    size_t length = size;
    size_t attr_length = strlen(attribute);
    if (attr_length > MOBI_ATTRNAME_MAXSIZE) {
        debug_print("Attribute too long: %zu\n", attr_length);
        return SIZE_MAX;
    }
    char attr[MOBI_ATTRNAME_MAXSIZE + 2];
    strcpy(attr, attribute);
    strcat(attr, "=");
    attr_length++;
    if (size < attr_length) {
        return SIZE_MAX;
    }
    /* FIXME: search may start inside tag, so it is a safer option */
    unsigned char last_border = '\0';
    do {
        if (*data == '<' || *data == '>') {
            last_border = *data;
        }
        if (length > attr_length + 1 && memcmp(data, attr, attr_length) == 0) {
            /* found match */
            size_t offset = size - length;
            if (last_border == '>') {
                /* We are in tag contents */
                data += attr_length;
                length -= attr_length - 1;
                continue;
            }
            /* previous character should be white space or opening tag */
            if (offset > 0) {
                if (data[-1] != '<' && !isspace(data[-1])) {
                    data += attr_length;
                    length -= attr_length - 1;
                    continue;
                }
            }
            /* now go forward */
            data += attr_length;
            length -= attr_length;
            unsigned char separator;
            if (*data != '\'' && *data != '"') {
                if (only_quoted) {
                    continue;
                }
                separator = ' ';
            } else {
                separator = *data;
                data++;
                length--;
            }
            size_t j;
            for (j = 0; j < MOBI_ATTRVALUE_MAXSIZE && length && *data != separator; j++) {
                *value++ = (char) *data++;
                length--;
            }
            *value = '\0';
            /* return offset to the beginning of the attribute value string */
            return size - length - j;
        }
        data++;
    } while (--length);
    value[0] = '\0';
    return SIZE_MAX;
}

/**
 @brief Get offset of the given value of an "aid" attribute in a given part
 
 @param[in] aid String value of "aid" attribute
 @param[in] html MOBIPart html part
 @return Offset from the beginning of the html part data, SIZE_MAX on failure
 */
size_t mobi_get_aid_offset(const MOBIPart *html, const char *aid) {
    size_t length = html->size;
    const char *data = (char *) html->data;
    const size_t aid_length = strlen(aid);
    const size_t attr_length = 5; /* "aid='" length */
    do {
        if (length > (aid_length + attr_length) && memcmp(data, "aid=", attr_length - 1) == 0) {
            data += attr_length;
            length -= attr_length;
            if (memcmp(data, aid, aid_length) == 0) {
                if (data[aid_length] == '\'' || data[aid_length] == '"') {
                    return html->size - length;
                }
            }
        }
        data++;
    } while (--length);
    return SIZE_MAX;
}

/**
 @brief Convert kindle:pos:fid:x:off:y to skeleton part number and offset from the beginning of the part
 
 @param[in,out] file_number Will be set to file number value
 @param[in,out] offset Offset from the beginning of the skeleton part
 @param[in] rawml MOBIRawml parsed records structure
 @param[in] pos_fid X value of pos:fid:x
 @param[in] pos_off X value of pos:off:x
 @return MOBI_RET status code (on success MOBI_SUCCESS)
 */
MOBI_RET mobi_get_offset_by_posoff(uint32_t *file_number, size_t *offset, const MOBIRawml *rawml, const size_t pos_fid, const size_t pos_off) {
    if (!rawml || !rawml->frag || !rawml->frag->entries ||
        !rawml->skel || !rawml->skel->entries) {
        debug_print("%s", "Initialization failed\n");
        return MOBI_INIT_FAILED;
    }
    MOBI_RET ret;
    if (pos_fid >= rawml->frag->entries_count) {
        debug_print("Entry for pos:fid:%zu doesn't exist\n", pos_fid);
        return MOBI_DATA_CORRUPT;
    }
    const MOBIIndexEntry *entry = &rawml->frag->entries[pos_fid];
    if (entry == NULL) {
        debug_print("Fragment entry for pos:fid:%zu not found\n", pos_fid);
        return MOBI_DATA_CORRUPT;
    }
    *offset = strtoul(entry->label, NULL, 10);
    uint32_t file_nr;
    ret = mobi_get_indxentry_tagvalue(&file_nr, entry, INDX_TAG_FRAG_FILE_NR);
    if (ret != MOBI_SUCCESS) {
        return ret;
    }
    if (file_nr >= rawml->skel->entries_count) {
        debug_print("Entry for skeleton part no %u doesn't exist\n", file_nr);
        return MOBI_DATA_CORRUPT;
        
    }
    const MOBIIndexEntry *skel_entry = &rawml->skel->entries[file_nr];
    uint32_t skel_position;
    ret = mobi_get_indxentry_tagvalue(&skel_position, skel_entry, INDX_TAG_SKEL_POSITION);
    if (ret != MOBI_SUCCESS) {
        return ret;
    }
    *offset -= skel_position;
    *offset += pos_off;
    *file_number = file_nr;
    return MOBI_SUCCESS;
}

/**
 @brief Get value of the closest "aid" attribute following given offset in a given part
 
 @param[in,out] aid String value of "aid" attribute
 @param[in] html MOBIPart html part
 @param[in] offset Offset from the beginning of the part data
 @return MOBI_RET status code (on success MOBI_SUCCESS)
 */
MOBI_RET mobi_get_aid_by_offset(char *aid, const MOBIPart *html, const size_t offset) {
    if (!aid || !html) {
        debug_print("Parameter error (aid (%p), html (%p)\n", aid, (void *) html);
        return MOBI_PARAM_ERR;
    }
    if (offset > html->size) {
        debug_print("Parameter error: offset (%zu) > part size (%zu)\n", offset, html->size);
        return MOBI_PARAM_ERR;
    }
    const unsigned char *data = html->data;
    data += offset;
    size_t length = html->size - offset + 1;
    
    size_t off = mobi_get_attribute_value(aid, data, length, "aid", true);
    if (off == SIZE_MAX) {
        return MOBI_DATA_CORRUPT;
    }
    return MOBI_SUCCESS;
}

/**
 @brief Get value of the closest "id" attribute following given offset in a given part
 
 @param[in,out] id String value of "id" attribute
 @param[in] html MOBIPart html part
 @param[in] offset Offset from the beginning of the part data
 @return MOBI_RET status code (on success MOBI_SUCCESS)
 */
MOBI_RET mobi_get_id_by_offset(char *id, const MOBIPart *html, const size_t offset) {
    if (!id || !html) {
        debug_print("Parameter error (id (%p), html (%p)\n", id, (void *) html);
        return MOBI_PARAM_ERR;
    }
    if (offset > html->size) {
        debug_print("Parameter error: offset (%zu) > part size (%zu)\n", offset, html->size);
        return MOBI_PARAM_ERR;
    }
    const unsigned char *data = html->data;
    data += offset;
    size_t length = html->size - offset + 1;
    
    size_t off = mobi_get_attribute_value(id, data, length, "id", true);
    if (off == SIZE_MAX) {
        id[0] = '\0';
        //return MOBI_DATA_CORRUPT;
    }
    return MOBI_SUCCESS;
}

/**
 @brief Convert kindle:pos:fid:x:off:y to html file number and closest "aid" attribute following the position
 
 @param[in,out] file_number Will be set to file number value
 @param[in,out] aid String value of "aid" attribute
 @param[in] rawml MOBIRawml parsed records structure
 @param[in] pos_fid X value of pos:fid:x
 @param[in] pos_off Y value of off:y
 @return MOBI_RET status code (on success MOBI_SUCCESS)
 */
MOBI_RET mobi_get_aid_by_posoff(uint32_t *file_number, char *aid, const MOBIRawml *rawml, const size_t pos_fid, const size_t pos_off) {
    size_t offset;
    MOBI_RET ret = mobi_get_offset_by_posoff(file_number, &offset, rawml, pos_fid, pos_off);
    if (ret != MOBI_SUCCESS) {
        return MOBI_DATA_CORRUPT;
    }
    const MOBIPart *html = mobi_get_part_by_uid(rawml, *file_number);
    if (html == NULL) {
        return MOBI_DATA_CORRUPT;
    }
    ret = mobi_get_aid_by_offset(aid, html, offset);
    if (ret != MOBI_SUCCESS) {
        return MOBI_DATA_CORRUPT;
    }
    return MOBI_SUCCESS;
}

/**
 @brief Convert kindle:pos:fid:x:off:y to html file number and closest "id" attribute following the position
 
 @param[in,out] file_number Will be set to file number value
 @param[in,out] id String value of "id" attribute
 @param[in] rawml MOBIRawml parsed records structure
 @param[in] pos_fid X value of pos:fid:x
 @param[in] pos_off Y value of off:y
 @return MOBI_RET status code (on success MOBI_SUCCESS)
 */
MOBI_RET mobi_get_id_by_posoff(uint32_t *file_number, char *id, const MOBIRawml *rawml, const size_t pos_fid, const size_t pos_off) {
    size_t offset;
    MOBI_RET ret = mobi_get_offset_by_posoff(file_number, &offset, rawml, pos_fid, pos_off);
    if (ret != MOBI_SUCCESS) {
        return MOBI_DATA_CORRUPT;
    }
    const MOBIPart *html = mobi_get_part_by_uid(rawml, *file_number);
    if (html == NULL) {
        return MOBI_DATA_CORRUPT;
    }
    ret = mobi_get_id_by_offset(id, html, offset);
    if (ret != MOBI_SUCCESS) {
        return MOBI_DATA_CORRUPT;
    }
    return MOBI_SUCCESS;
}

/**
 @brief Parse resource records (images, fonts etc), determine their type, link to rawml
 
 @param[in] m MOBIData structure with loaded Record(s) 0 headers
 @param[in,out] rawml Structure rawml->resources will be filled with parsed resources metadata and linked records data
 @return MOBI_RET status code (on success MOBI_SUCCESS)
 */
MOBI_RET mobi_reconstruct_resources(const MOBIData *m, MOBIRawml *rawml) {
    size_t first_res_seqnumber = mobi_get_first_resource_record(m);
    if (first_res_seqnumber == MOBI_NOTSET) {
        /* search all records */
        first_res_seqnumber = 0;
    }
    const MOBIPdbRecord *curr_record = mobi_get_record_by_seqnumber(m, first_res_seqnumber);
    if (curr_record == NULL) {
        debug_print("First resource record not found at %zu\n", first_res_seqnumber);
        return MOBI_DATA_CORRUPT;
    }
    rawml->resources = calloc(1, sizeof(MOBIPart));
    if (rawml->resources == NULL) {
        debug_print("%s", "Memory allocation for resources part failed\n");
        return MOBI_MALLOC_FAILED;
    }
    MOBIPart *curr_part = rawml->resources;
    size_t i = 0;
    int parts_count = 0;
    while (curr_record != NULL) {
        const MOBIFiletype filetype = mobi_determine_resource_type(curr_record);
        if (filetype == T_UNKNOWN) {
            curr_record = curr_record->next;
            i++;
            continue;
        }
        if (filetype == T_BREAK) {
            break;
        }
        if (parts_count > 0) {
            curr_part->next = calloc(1, sizeof(MOBIPart));
            if (curr_part->next == NULL) {
                debug_print("%s", "Memory allocation for flow part failed\n");
                return MOBI_MALLOC_FAILED;
            }
            curr_part = curr_part->next;
        }
        
        curr_part->data = curr_record->data;
        curr_part->size = curr_record->size;
        
        MOBI_RET ret;
        if (filetype == T_FONT) {
            ret = mobi_add_font_resource(curr_part);
            if (ret != MOBI_SUCCESS) {
                printf("Decoding font resource failed\n");
                return ret;
            }
        } else if (filetype == T_AUDIO) {
            ret = mobi_add_audio_resource(curr_part);
            if (ret != MOBI_SUCCESS) {
                printf("Decoding audio resource failed\n");
                return ret;
            }
        } else if (filetype == T_VIDEO) {
            ret = mobi_add_video_resource(curr_part);
            printf("Decoding video resource failed\n");
            if (ret != MOBI_SUCCESS) {
                return ret;
            }
        } else {
            curr_part->type = filetype;
        }
        
        curr_part->uid = i;
        curr_part->next = NULL;
        curr_record = curr_record->next;
        i++;
        parts_count++;
    }
    if (parts_count == 0) {
        free(rawml->resources);
        rawml->resources = NULL;
    }
    return MOBI_SUCCESS;
}

/**
 @brief Parse Replica Print ebook (azw4). Extract pdf.
 @todo Parse remaining data from the file
 
 @param[in,out] pdf Memory area will be filled with extracted pdf data
 @param[in] text Raw decompressed text to be parsed
 @param[in,out] length Text length. Will be updated with pdf_length on return
 @return MOBI_RET status code (on success MOBI_SUCCESS)
 */
MOBI_RET mobi_process_replica(unsigned char *pdf, const char *text, size_t *length) {
    MOBI_RET ret = MOBI_SUCCESS;
    MOBIBuffer *buf = buffer_init_null(*length);
    if (buf == NULL) {
        return MOBI_MALLOC_FAILED;
    }
    buf->data = (unsigned char*) text;
    buf->offset = 12;
    size_t pdf_offset = buffer_get32(buf); /* offset 12 */
    size_t pdf_length = buffer_get32(buf); /* 16 */
    if (pdf_length > *length) {
        debug_print("PDF size from replica header too large: %zu", pdf_length);
        buffer_free_null(buf);
        return MOBI_DATA_CORRUPT;
    }
    buf->offset = pdf_offset;
    buffer_getraw(pdf, buf, pdf_length);
    ret = buf->error;
    buffer_free_null(buf);
    *length = pdf_length;
    return ret;
}

/**
 @brief Parse raw text into flow parts
 
 @param[in,out] rawml Structure rawml->flow will be filled with parsed flow text parts
 @param[in] text Raw decompressed text to be parsed
 @param[in] length Text length
 @return MOBI_RET status code (on success MOBI_SUCCESS)
 */
MOBI_RET mobi_reconstruct_flow(MOBIRawml *rawml, const char *text, const size_t length) {
    /* KF8 */
    if (rawml->fdst != NULL) {
        rawml->flow = calloc(1, sizeof(MOBIPart));
        if (rawml->flow == NULL) {
            debug_print("%s", "Memory allocation for flow part failed\n");
            return MOBI_MALLOC_FAILED;
        }
        /* split text into fdst structure parts */
        MOBIPart *curr = rawml->flow;
        size_t i = 0;
        const size_t section_count = rawml->fdst->fdst_section_count;
        while (i < section_count) {
            if (i > 0) {
                curr->next = calloc(1, sizeof(MOBIPart));
                if (curr->next == NULL) {
                    debug_print("%s", "Memory allocation for flow part failed\n");
                    return MOBI_MALLOC_FAILED;
                }
                curr = curr->next;
            }
            const uint32_t section_start = rawml->fdst->fdst_section_starts[i];
            const uint32_t section_end = rawml->fdst->fdst_section_ends[i];
            const size_t section_length = section_end - section_start;
            unsigned char *section_data = malloc(section_length);
            if (section_data == NULL) {
                debug_print("%s", "Memory allocation failed\n");
                return MOBI_MALLOC_FAILED;
            }
            memcpy(section_data, (text + section_start), section_length);
            curr->uid = i;
            curr->data = section_data;
            curr->type = mobi_determine_flowpart_type(rawml, i);
            curr->size = section_length;
            curr->next = NULL;
            i++;
        }
    } else {
        /* No FDST or FDST parts count = 1 */
        /* single flow part */
        rawml->flow = calloc(1, sizeof(MOBIPart));
        if (rawml->flow == NULL) {
            debug_print("%s", "Memory allocation for flow part failed\n");
            return MOBI_MALLOC_FAILED;
        }
        MOBIPart *curr = rawml->flow;
        size_t section_length = 0;
        MOBIFiletype section_type = T_HTML;
        unsigned char *section_data;
        /* check if raw text is Print Replica */
        if (memcmp(text, REPLICA_MAGIC, 4) == 0) {
            debug_print("%s", "Print Replica book\n");
            /* print replica */
            unsigned char *pdf = malloc(length);
            section_length = length;
            section_type = T_PDF;
            const MOBI_RET ret = mobi_process_replica(pdf, text, &section_length);
            if (ret != MOBI_SUCCESS) {
                free(pdf);
                return ret;
            }
            section_data = malloc(section_length);
            if (section_data == NULL) {
                debug_print("%s", "Memory allocation failed\n");
                free(pdf);
                return MOBI_MALLOC_FAILED;
            }
            memcpy(section_data, pdf, section_length);
            free(pdf);
        } else {
            /* text data */
            section_length = strlen(text);
            section_data = malloc(section_length);
            if (section_data == NULL) {
                debug_print("%s", "Memory allocation failed\n");
                return MOBI_MALLOC_FAILED;
            }
            memcpy(section_data, text, section_length);
        }
        curr->uid = 0;
        curr->data = section_data;
        curr->type = section_type;
        curr->size = section_length;
        curr->next = NULL;
    }
    return MOBI_SUCCESS;
}

/**
 @brief Parse raw html into html parts. Use index entries if present to parse file
 
 @param[in,out] rawml Structure rawml->markup will be filled with reconstructed html parts
 @return MOBI_RET status code (on success MOBI_SUCCESS)
 */
MOBI_RET mobi_reconstruct_parts(MOBIRawml *rawml) {
    MOBI_RET ret;
    if (rawml->flow == NULL) {
        debug_print("%s", "Flow structure not initialized\n");
        return MOBI_INIT_FAILED;
    }
    /* take first part, xhtml */
    MOBIBuffer *buf = buffer_init_null(rawml->flow->size);
    buf->data = rawml->flow->data;
    rawml->markup = calloc(1, sizeof(MOBIPart));
    if (rawml->markup == NULL) {
        debug_print("%s", "Memory allocation for markup part failed\n");
        buffer_free_null(buf);
        return MOBI_MALLOC_FAILED;
    }
    MOBIPart *curr = rawml->markup;
    /* not skeleton data, just copy whole part to markup */
    if (rawml->skel == NULL) {
        unsigned char *data = malloc(buf->maxlen);
        if (data == NULL) {
            debug_print("%s", "Memory allocation failed\n");
            buffer_free_null(buf);
            return MOBI_MALLOC_FAILED;
        }
        memcpy(data, buf->data, buf->maxlen);
        curr->uid = 0;
        curr->size = buf->maxlen;
        curr->data = data;
        curr->type = rawml->flow->type;
        curr->next = NULL;
        buffer_free_null(buf);
        return MOBI_SUCCESS;
    }
    /* parse skeleton data */
    size_t i = 0, j = 0;
    while (i < rawml->skel->entries_count) {
        const MOBIIndexEntry *entry = &rawml->skel->entries[i];
        uint32_t fragments_count;
        ret = mobi_get_indxentry_tagvalue(&fragments_count, entry, INDX_TAG_SKEL_COUNT);
        if (ret != MOBI_SUCCESS) {
            buffer_free_null(buf);
            return ret;
        }
        uint32_t skel_position;
        ret = mobi_get_indxentry_tagvalue(&skel_position, entry, INDX_TAG_SKEL_POSITION);
        if (ret != MOBI_SUCCESS) {
            buffer_free_null(buf);
            return ret;
        }
        uint32_t skel_length;
        ret = mobi_get_indxentry_tagvalue(&skel_length, entry, INDX_TAG_SKEL_LENGTH);
        if (ret != MOBI_SUCCESS) {
            buffer_free_null(buf);
            return ret;
        }
        debug_print("%zu\t%s\t%i\t%i\t%i\n", i, entry->label, fragments_count, skel_position, skel_length);
        char *skel_text = malloc(skel_length + 1);
        buf->offset = skel_position;
        buffer_getstring(skel_text, buf, skel_length);
        while (fragments_count--) {
            entry = &rawml->frag->entries[j];
            uint32_t insert_position = (uint32_t) strtoul(entry->label, NULL, 10);
            insert_position -= skel_position;
            uint32_t cncx_offset;
            ret = mobi_get_indxentry_tagvalue(&cncx_offset, entry, INDX_TAG_FRAG_AID_CNCX);
            if (ret != MOBI_SUCCESS) {
                free(skel_text);
                buffer_free_null(buf);
                return ret;
            }
            uint32_t file_number;
            ret = mobi_get_indxentry_tagvalue(&file_number, entry, INDX_TAG_FRAG_FILE_NR);
            if (ret != MOBI_SUCCESS) {
                free(skel_text);
                buffer_free_null(buf);
                return ret;
            }
            uint32_t seq_number;
            ret = mobi_get_indxentry_tagvalue(&seq_number, entry, INDX_TAG_FRAG_SEQUENCE_NR);
            if (ret != MOBI_SUCCESS) {
                free(skel_text);
                buffer_free_null(buf);
                return ret;
            }
            uint32_t frag_position;
            ret = mobi_get_indxentry_tagvalue(&frag_position, entry, INDX_TAG_FRAG_POSITION);
            if (ret != MOBI_SUCCESS) {
                free(skel_text);
                buffer_free_null(buf);
                return ret;
            }
            uint32_t frag_length;
            ret = mobi_get_indxentry_tagvalue(&frag_length, entry, INDX_TAG_FRAG_LENGTH);
            if (ret != MOBI_SUCCESS) {
                free(skel_text);
                buffer_free_null(buf);
                return ret;
            }
            /* FIXME: aid_text is unused */
            const MOBIPdbRecord *cncx_record = rawml->frag->cncx_record;
            char *aid_text = mobi_get_cncx_string(cncx_record, cncx_offset);
            if (file_number != i) {
                debug_print("%s", "SKEL part number and fragment sequence number don't match\n");
                free(aid_text);
                free(skel_text);
                buffer_free_null(buf);
                return MOBI_DATA_CORRUPT;
            }
            debug_print("posfid[%zu]\t%i\t%i\t%s\t%i\t%i\t%i\t%i\n", j, insert_position, cncx_offset, aid_text, file_number, seq_number, frag_position, frag_length);
            free(aid_text);
            char *tmp = realloc(skel_text, (skel_length + frag_length + 1));
            if (tmp == NULL) {
                free(skel_text);
                buffer_free_null(buf);
                return MOBI_MALLOC_FAILED;
            }
            skel_text = tmp;
            size_t skel_end_length = skel_length - insert_position;
            char skel_text_end[skel_end_length + 1];
            strncpy(skel_text_end, skel_text + insert_position, skel_end_length);
            skel_text_end[skel_end_length] = '\0';
            skel_text[insert_position] = '\0';
            buffer_appendstring(skel_text, buf, frag_length);
            skel_length += frag_length;
            strncat(skel_text, skel_text_end, skel_length + 1);
            j++;
            
        }
        if (i > 0) {
            curr->next = calloc(1, sizeof(MOBIPart));
            if (curr->next == NULL) {
                debug_print("%s", "Memory allocation for markup part failed\n");
                buffer_free_null(buf);
                return MOBI_MALLOC_FAILED;
            }
            curr = curr->next;
        }
        curr->uid = i;
        curr->size = skel_length;
        curr->data = (unsigned char *) skel_text;
        curr->type = T_HTML;
        curr->next = NULL;
        i++;
    }
    buffer_free_null(buf);
    return MOBI_SUCCESS;
}

/**
 @brief Scan html part and build array of filepos link target offsets
 
 @param[in,out] links MOBIArray structure for link target offsets array
 @param[in] part MOBIPart html part structure
 @return MOBI_RET status code (on success MOBI_SUCCESS)
 */
MOBI_RET mobi_get_filepos_array(MOBIArray *links, const MOBIPart *part) {
    if (!links || !part) {
        return MOBI_INIT_FAILED;
    }
    size_t offset = 0;
    size_t size = part->size;
    unsigned char *data = part->data;
    while (true) {
        char val[MOBI_ATTRVALUE_MAXSIZE];
        size -= offset;
        data += offset;
        offset = mobi_get_attribute_value(val, data, size, "filepos", false);
        if (offset == SIZE_MAX) { break; }
        size_t filepos = strtoul(val, NULL, 10);
        if (filepos > UINT32_MAX || filepos == 0) {
            debug_print("Filepos out of range: %zu\n", filepos);
            continue;
        }
        MOBI_RET ret = array_insert(links, (uint32_t) filepos);
        if (ret != MOBI_SUCCESS) {
            return ret;
        }
    }
    return MOBI_SUCCESS;
}

/**
 @brief Scan ncx part and build array of filepos link target offsets.
 
 @param[in,out] links MOBIArray structure for link target offsets array
 @param[in] part MOBIPart html part structure
 @return MOBI_RET status code (on success MOBI_SUCCESS)
 */
MOBI_RET mobi_get_ncx_filepos_array(MOBIArray *links, const MOBIPart *part) {
    if (!links || !part) {
        return MOBI_PARAM_ERR;
    }
    while ((part = part->next) != NULL) {
        if (part->type == T_NCX) {
            size_t offset = 0;
            size_t size = part->size;
            unsigned char *data = part->data;
            while (true) {
                char val[MOBI_ATTRVALUE_MAXSIZE];
                size -= offset;
                data += offset;
                offset = mobi_get_attribute_value(val, data, size, "src", false);
                if (offset == SIZE_MAX) { break; }
                /* part00000.html#0000000000 */
                uint32_t filepos = 0;
                sscanf(val + 15, "%10u", &filepos);
                MOBI_RET ret = array_insert(links, filepos);
                if (ret != MOBI_SUCCESS) {
                    return ret;
                }
            }
        }
    }
    return MOBI_SUCCESS;
}

/**
 @brief Replace kindle:pos link with html href
 
 @param[in,out] link Memory area which will be filled with "part00000.html#customid", including quotation marks
 @param[in] rawml Structure rawml will be filled with reconstructed parts and resources
 @param[in] value String kindle:pos:fid:0000:off:0000000000, without quotation marks
 @return MOBI_RET status code (on success MOBI_SUCCESS)
 */
MOBI_RET mobi_posfid_to_link(char *link, const MOBIRawml *rawml, const char *value) {
    /* "kindle:pos:fid:0000:off:0000000000" */
    /* extract fid and off */
    value += 15; /* strlen("kindle:pos:fid:"); */
    char str_fid[4 + 1];
    strncpy(str_fid, value, 4);
    str_fid[4] = '\0';
    char str_off[10 + 1];
    value += 9; /* strlen("0001:off:"); */
    strncpy(str_off, value, 10);
    str_off[10] = '\0';
    
    /* get file number and id value */
    uint32_t pos_off, pos_fid;
    MOBI_RET ret = mobi_base32_decode(&pos_off, str_off);
    if (ret != MOBI_SUCCESS) {
        return ret;
    }
    ret = mobi_base32_decode(&pos_fid, str_fid);
    if (ret != MOBI_SUCCESS) {
        return ret;
    }
    uint32_t part_id;
    char id[MOBI_ATTRVALUE_MAXSIZE + 1];
    ret = mobi_get_id_by_posoff(&part_id, id, rawml, pos_fid, pos_off);
    if (ret != MOBI_SUCCESS) {
        return ret;
    }
    /* FIXME: pos_off == 0 means top of file? */
    if (pos_off) {
        snprintf(link, MOBI_ATTRVALUE_MAXSIZE, "\"part%05u.html#%s\"", part_id, id);
    } else {
        snprintf(link, MOBI_ATTRVALUE_MAXSIZE, "\"part%05u.html\"", part_id);
    }
    return MOBI_SUCCESS;
}

/**
 @brief Replace kindle:flow link with html href
 
 @param[in,out] link Memory area which will be filled with "part00000.ext", including quotation marks
 @param[in] rawml Structure rawml will be filled with reconstructed parts and resources
 @param[in] value String kindle:flow:0000?mime=type, without quotation marks
 @return MOBI_RET status code (on success MOBI_SUCCESS)
 */
MOBI_RET mobi_flow_to_link(char *link, const MOBIRawml *rawml, const char *value) {
    /* "kindle:flow:0000?mime=" */
    value += 12; /* strlen("kindle:flow:"); */
    char str_fid[4 + 1];
    strncpy(str_fid, value, 4);
    str_fid[4] = '\0';
    
    /* get file number */
    uint32_t part_id;
    MOBI_RET ret = mobi_base32_decode(&part_id, str_fid);
    if (ret != MOBI_SUCCESS) {
        return ret;
    }
    MOBIPart *flow = mobi_get_flow_by_uid(rawml, part_id);
    MOBIFileMeta meta = mobi_get_filemeta_by_type(flow->type);
    char *extension = meta.extension;
    snprintf(link, MOBI_ATTRVALUE_MAXSIZE, "\"flow%05u.%s\"", part_id, extension);
    return MOBI_SUCCESS;
}

/**
 @brief Replace kindle:embed link with html href
 
 @param[in,out] link Memory area which will be filled with "resource00000.ext", including quotation marks
 @param[in] rawml Structure rawml will be filled with reconstructed parts and resources
 @param[in] value String kindle:embed:0000?mime=type, with optional quotation marks
 @return MOBI_RET status code (on success MOBI_SUCCESS)
 */
MOBI_RET mobi_embed_to_link(char *link, const MOBIRawml *rawml, const char *value) {
    /* "kindle:embed:0000?mime=" */
    /* skip quotation marks or spaces */
    while (*value == '"' || *value == '\'' || isspace(*value)) {
        value++;
    }
    value += strlen("kindle:embed:");
    char str_fid[4 + 1];
    strncpy(str_fid, value, 4);
    str_fid[4] = '\0';
    
    /* get file number */
    uint32_t part_id;
    MOBI_RET ret = mobi_base32_decode(&part_id, str_fid);
    if (ret != MOBI_SUCCESS) {
        return ret;
    }
    part_id--;
    MOBIPart *resource = mobi_get_resource_by_uid(rawml, part_id);
    MOBIFileMeta meta = mobi_get_filemeta_by_type(resource->type);
    char *extension = meta.extension;
    snprintf(link, MOBI_ATTRVALUE_MAXSIZE, "\"resource%05u.%s\"", part_id, extension);
    return MOBI_SUCCESS;
}

/**
 @brief Structure for links reconstruction.
 
 Linked list of Fragment structures forms whole document part
 */
typedef struct MOBIFragment {
    size_t raw_offset; /**< fragment offset in raw markup, SIZE_MAX if not present in original markup */
    unsigned char *fragment; /**< Fragment data */
    size_t size; /**< Fragment size */
    bool is_malloc; /**< Is it needed to free this fragment or is it just an alias to part data */
    struct MOBIFragment *next; /**< Link to next fragment */
} MOBIFragment;


/**
 @brief Allocate fragment, fill with data and return
 
 @param[in] raw_offset Fragment offset in raw markup, 
            SIZE_MAX if not present in original markup
 @param[in] fragment Fragment data
 @param[in] size Size data
 @param[in] is_malloc is_maloc data
 @return Fragment structure filled with data
 */
static MOBIFragment * mobi_list_init(size_t raw_offset, unsigned char *fragment, const size_t size, const bool is_malloc) {
    MOBIFragment *curr = calloc(1, sizeof(MOBIFragment));
    if (curr == NULL) {
        return NULL;
    }
    curr->raw_offset = raw_offset;
    curr->fragment = fragment;
    curr->size = size;
    curr->is_malloc = is_malloc;
    return curr;
}

/**
 @brief Allocate fragment, fill with data, append to linked list
 
 @param[in] raw_offset Fragment offset in raw markup,
            SIZE_MAX if not present in original markup
 @param[in] curr Last fragment in linked list
 @param[in] fragment Fragment data
 @param[in] size Size data
 @param[in] is_malloc is_maloc data
 @return Fragment structure filled with data
 */
static MOBIFragment * mobi_list_add(MOBIFragment *curr, size_t raw_offset, unsigned char *fragment, const size_t size, const bool is_malloc) {
    if (!curr) {
        return mobi_list_init(raw_offset, fragment, size, is_malloc);
    }
    curr->next = calloc(1, sizeof(MOBIFragment));
    if (curr->next == NULL) {
        return NULL;
    }
    MOBIFragment *next = curr->next;
    next->raw_offset = raw_offset;
    next->fragment = fragment;
    next->size = size;
    next->is_malloc = is_malloc;
    return next;
}

/**
 @brief Allocate fragment, fill with data, 
        insert into linked list at given offset
 
 Starts to search for offset at curr fragment.
 
 @param[in] raw_offset Fragment offset in raw markup,
            SIZE_MAX if not present in original markup
 @param[in] curr Fragment where search starts
 @param[in] fragment Fragment data
 @param[in] size Size data
 @param[in] is_malloc is_maloc data
 @param[in] offset offset where new chunk will be inserted
 @return Fragment structure filled with data
 */
static MOBIFragment * mobi_list_insert(MOBIFragment *curr, size_t raw_offset, unsigned char *fragment, const size_t size, const bool is_malloc, const size_t offset) {
    MOBIFragment *prev = NULL;
    while (curr) {
        if (curr->raw_offset != SIZE_MAX && curr->raw_offset <= offset && curr->raw_offset + curr->size >= offset ) {
            break;
        }
        prev = curr;
        curr = curr->next;
    }
    if (!curr) {
        /* FIXME: return value is same as with malloc error */
        debug_print("Offset not found: %zu\n", offset);
        return NULL;
    }
    MOBIFragment *new = calloc(1, sizeof(MOBIFragment));
    if (new == NULL) {
        return NULL;
    }
    new->raw_offset = raw_offset;
    new->fragment = fragment;
    new->size = size;
    new->is_malloc = is_malloc;
    MOBIFragment *new2 = NULL;
    if (curr->raw_offset == offset) {
        /* prepend chunk */
        if (prev) {
            prev->next = new;
            new->next = curr;
        } else {
            /* save curr */
            MOBIFragment tmp;
            tmp.raw_offset = curr->raw_offset;
            tmp.fragment = curr->fragment;
            tmp.size = curr->size;
            tmp.is_malloc = curr->is_malloc;
            tmp.next = curr->next;
            /* move new to curr */
            curr->raw_offset = new->raw_offset;
            curr->fragment = new->fragment;
            curr->size = new->size;
            curr->is_malloc = new->is_malloc;
            curr->next = new;
            /* restore tmp to new */
            new->raw_offset = tmp.raw_offset;
            new->fragment = tmp.fragment;
            new->size = tmp.size;
            new->is_malloc = tmp.is_malloc;
            new->next = tmp.next;
            return curr;
        }
    } else if (curr->raw_offset + curr->size == offset) {
        /* append chunk */
        new->next = curr->next;
        curr->next = new;
    } else {
        /* split fragment and insert new chunk */
        new2 = calloc(1, sizeof(MOBIFragment));
        if (new2 == NULL) {
            free(new);
            return NULL;
        }
        size_t rel_offset = offset - curr->raw_offset;
        new2->next = curr->next;
        new2->size = curr->size - rel_offset;
        new2->raw_offset = offset;
        new2->fragment = curr->fragment + rel_offset;
        new2->is_malloc = false;
        curr->next = new;
        curr->size = rel_offset;
        new->next = new2;
    }
    return new;
}

/**
 @brief Delete fragment from linked list
 
 @param[in] curr Fragment to be deleted
 @return Next fragment in the linked list or NULL if absent
 */
static MOBIFragment * mobi_list_del(MOBIFragment *curr) {
    MOBIFragment *del = curr;
    curr = curr->next;
    if (del->is_malloc) {
        free(del->fragment);
    }
    free(del);
    del = NULL;
    return curr;
}

/**
 @brief Delete all fragments from linked list
 
 @param[in] first First fragment from the list
 */
static void mobi_list_del_all(MOBIFragment *first) {
    while (first) {
        first = mobi_list_del(first);
    }
}

/**
 @brief Replace offset-links with html-links in KF8 markup
 
 @param[in,out] rawml Structure rawml will be filled with reconstructed parts and resources
 @return MOBI_RET status code (on success MOBI_SUCCESS)
 */
MOBI_RET mobi_reconstruct_links_kf8(const MOBIRawml *rawml) {
    MOBIResult result;
    
    typedef struct NEWData {
        size_t part_group;
        size_t part_uid;
        MOBIFragment *list;
        size_t size;
        struct NEWData *next;
    } NEWData;
    
    NEWData *partdata = NULL;
    NEWData *curdata = NULL;
    MOBIPart *parts[] = {
        rawml->markup, /* html files */
        rawml->flow->next /* css, skip first unparsed html part */
    };
    size_t i;
    for (i = 0; i < 2; i++) {
        MOBIPart *part = parts[i];
        while (part) {
            unsigned char *data_in = part->data;
            result.start = part->data;
            const unsigned char *data_end = part->data + part->size;
            MOBIFragment *first = NULL;
            MOBIFragment *curr = NULL;
            size_t part_size = 0;
            while (true) {
                mobi_search_links_kf8(&result, result.start, data_end, part->type);
                if (result.start == NULL) {
                    break;
                }
                char *value = (char *) result.value;
                unsigned char *data_cur = result.start;
                char *target = NULL;
                if (data_cur < data_in) {
                    mobi_list_del_all(first);
                    return MOBI_DATA_CORRUPT;
                }
                size_t size = (size_t) (data_cur - data_in);
                char link[MOBI_ATTRVALUE_MAXSIZE + 1];
                if ((target = strstr(value, "kindle:pos:fid:")) != NULL) {
                    /* "kindle:pos:fid:0001:off:0000000000" */
                    /* replace link with href="part00000.html#00" */
                    MOBI_RET ret = mobi_posfid_to_link(link, rawml, target);
                    if (ret != MOBI_SUCCESS) {
                        mobi_list_del_all(first);
                        return ret;
                    }
                } else if ((target = strstr(value, "kindle:flow:")) != NULL) {
                    /* kindle:flow:0000?mime=text/css */
                    /* replace link with href="flow00000.ext" */
                    MOBI_RET ret = mobi_flow_to_link(link, rawml, target);
                    if (ret != MOBI_SUCCESS) {
                        mobi_list_del_all(first);
                        return ret;
                    }
                } else if ((target = strstr(value, "kindle:embed:")) != NULL) {
                    /* kindle:embed:0000?mime=image/jpg */
                    /* replace link with href="resource00000.ext" */
                    MOBI_RET ret = mobi_embed_to_link(link, rawml, target);
                    if (ret != MOBI_SUCCESS) {
                        mobi_list_del_all(first);
                        return ret;
                    }
                }
                if (target) {
                    /* first chunk */
                    curr = mobi_list_add(curr, (size_t) (data_in - part->data ), data_in, size, false);
                    if (curr == NULL) {
                        mobi_list_del_all(first);
                        return MOBI_MALLOC_FAILED;
                    }
                    if (!first) { first = curr; }
                    part_size += curr->size;
                    /* second chunk */
                    /* strip quotes if is_url */
                    curr = mobi_list_add(curr, SIZE_MAX,
                                         (unsigned char *) strdup(link + result.is_url),
                                         strlen(link) - 2 * result.is_url, true);
                    if (curr == NULL) {
                        mobi_list_del_all(first);
                        return MOBI_MALLOC_FAILED;
                    }
                    part_size += curr->size;
                    data_in = result.end;
                }
            }
            if (first && first->fragment) {
                /* last chunk */
                if (part->data + part->size < data_in) {
                    mobi_list_del_all(first);
                    return MOBI_DATA_CORRUPT;
                }
                size_t size = (size_t) (part->data + part->size - data_in);
                curr = mobi_list_add(curr, (size_t) (data_in - part->data ), data_in, size, false);
                if (curr == NULL) {
                    mobi_list_del_all(first);
                    return MOBI_MALLOC_FAILED;
                }
                part_size += curr->size;
                /* save */
                if (!curdata) {
                    curdata = calloc(1, sizeof(NEWData));
                    partdata = curdata;
                } else {
                    curdata->next = calloc(1, sizeof(NEWData));
                    curdata = curdata->next;
                }
                curdata->part_group = i;
                curdata->part_uid = part->uid;
                curdata->list = first;
                curdata->size = part_size;
            }
            part = part->next;
        }
    }
    /* now update parts */
    for (i = 0; i < 2; i++) {
        MOBIPart *part = parts[i];
        while (part) {
            if (partdata && part->uid == partdata->part_uid && i == partdata->part_group) {
                unsigned char *new_data = malloc((size_t) partdata->size);
                unsigned char *data_out = new_data;
                MOBIFragment *fragdata = partdata->list;
                while (fragdata) {
                    memcpy(data_out, fragdata->fragment, fragdata->size);
                    data_out += fragdata->size;
                    fragdata = mobi_list_del(fragdata);
                }
                free(part->data);
                part->data = new_data;
                part->size = (size_t) partdata->size;
                NEWData *partused = partdata;
                partdata = partdata->next;
                free(partused);
            }
            part = part->next;
        }
    }
    return MOBI_SUCCESS;
}

/**
 @brief Replace offset-links with html-links in KF7 markup
 
 @param[in,out] rawml Structure rawml will be filled with reconstructed parts and resources
 @return MOBI_RET status code (on success MOBI_SUCCESS)
 */
MOBI_RET mobi_reconstruct_links_kf7(const MOBIRawml *rawml) {
    MOBIResult result;
    MOBIArray *links = array_init(25);
    if (links == NULL) {
        return MOBI_MALLOC_FAILED;
    }
    MOBIPart *part = rawml->markup;
    /* get array of link target offsets */
    MOBI_RET ret = mobi_get_filepos_array(links, part);
    if (ret != MOBI_SUCCESS) {
        array_free(links);
        return ret;
    }
    ret = mobi_get_ncx_filepos_array(links, part);
    if (ret != MOBI_SUCCESS) {
        array_free(links);
        return ret;
    }
    unsigned char *data_in = part->data;
    MOBIFragment *first = NULL;
    MOBIFragment *curr = NULL;
    size_t new_size = 0;
    if (array_size(links) > 0) {
        array_sort(links, true);
        /* build MOBIResult list */
        result.start = part->data;
        const unsigned char *data_end = part->data + part->size;
        while (true) {
            mobi_search_links_kf7(&result, result.start, data_end);
            if (result.start == NULL) {
                break;
            }
            char *attribute = (char *) result.value;
            unsigned char *data_cur = result.start;
            char link[MOBI_ATTRVALUE_MAXSIZE];
            const char *numbers = "0123456789";
            char *value = strpbrk(attribute, numbers);
            if (value == NULL) {
                debug_print("Unknown link target: %s\n", attribute);
                data_in = result.end;
                continue;
            }
            size_t target;
            switch (attribute[0]) {
                case 'f':
                    /* filepos=0000000000 */
                    /* replace link with href="#0000000000" */
                    target = strtoul(value, NULL, 10);
                    snprintf(link, MOBI_ATTRVALUE_MAXSIZE, "href=\"#%010u\"", (uint32_t)target);
                    break;
                case 'r':
                    /* recindex="00000" */
                    /* replace link with src="resource00000.ext" */
                    target = strtoul(value, NULL, 10);
                    if (target > 0) {
                        target--;
                    }
                    MOBIFiletype filetype = mobi_get_resourcetype_by_uid(rawml, target);
                    MOBIFileMeta filemeta = mobi_get_filemeta_by_type(filetype);
                    snprintf(link, MOBI_ATTRVALUE_MAXSIZE, "src=\"resource%05u.%s\"", (uint32_t) target, filemeta.extension);
                    break;
                default:
                    debug_print("Unknown link target: %s\n", attribute);
                    data_in = result.end;
                    continue;
            }
            
            /* first chunk */
            if (data_cur < data_in) {
                mobi_list_del_all(first);
                return MOBI_DATA_CORRUPT;
            }
            size_t size = (size_t) (data_cur - data_in);
            size_t raw_offset = (size_t) (data_in - part->data);
            curr = mobi_list_add(curr, raw_offset, data_in, size, false);
            if (curr == NULL) {
                mobi_list_del_all(first);
                return MOBI_MALLOC_FAILED;
            }
            if (!first) { first = curr; }
            new_size += curr->size;
            /* second chunk */
            curr = mobi_list_add(curr, SIZE_MAX,
                                 (unsigned char *) strdup(link),
                                 strlen(link), true);
            if (curr == NULL) {
                mobi_list_del_all(first);
                return MOBI_MALLOC_FAILED;
            }
            new_size += curr->size;
            data_in = result.end;
        }
    }
    if (first) {
        /* last chunk */
        if (part->data + part->size < data_in) {
            mobi_list_del_all(first);
            return MOBI_DATA_CORRUPT;
        }
        size_t size = (size_t) (part->data + part->size - data_in);
        size_t raw_offset = (size_t) (data_in - part->data);
        curr = mobi_list_add(curr, raw_offset, data_in, size, false);
        if (curr == NULL) {
            mobi_list_del_all(first);
            return MOBI_MALLOC_FAILED;
        }
        new_size += curr->size;
    } else {
        /* add whole part as one fragment */
        first = mobi_list_add(first, 0, part->data, part->size, false);
        new_size += first->size;
    }
    /* insert chunks from links array */
    curr = first;
    size_t i = 0;
    while (i < links->size) {
        const uint32_t offset = links->data[i];
        char anchor[MOBI_ATTRVALUE_MAXSIZE];
        snprintf(anchor, MOBI_ATTRVALUE_MAXSIZE, "<a id=\"%010u\"></a>", offset);
        curr = mobi_list_insert(curr, SIZE_MAX,
                               (unsigned char *) strdup(anchor),
                                strlen(anchor), true, offset);
        if (curr == NULL) {
            mobi_list_del_all(first);
            return MOBI_MALLOC_FAILED;
        }
        new_size += curr->size;
        i++;
    }
    array_free(links);
    /* insert dictionary markup if present */
    if (rawml->orth) {
        curr = first;
        i = 0;
        const size_t count = rawml->orth->entries_count;
        char *start_tag;
        const char *start_tag1 = "<idx:entry><idx:orth value=\"%s\"></idx:orth></idx:entry>";
        const char *start_tag2 = "<idx:entry scriptable=\"yes\"><idx:orth value=\"%s\"></idx:orth>";
        const char *end_tag = "</idx:entry>";
        const size_t start_tag1_len = strlen(start_tag1) - 2;
        const size_t start_tag2_len = strlen(start_tag2) - 2;
        const size_t end_tag_len = strlen(end_tag);
        uint32_t prev_startpos = 0;
        while (i < count) {
            const MOBIIndexEntry *orth_entry = &rawml->orth->entries[i];
            const char *label = orth_entry->label;
            uint32_t entry_startpos;
            uint32_t entry_textlen = 0;
            ret = mobi_get_indxentry_tagvalue(&entry_startpos, orth_entry, INDX_TAG_ORTH_STARTPOS);
            if (ret != MOBI_SUCCESS) {
                mobi_list_del_all(first);
                return ret;
            }
            char *entry_text;
            size_t entry_length;
            mobi_get_indxentry_tagvalue(&entry_textlen, orth_entry, INDX_TAG_ORTH_ENDPOS);
            if (entry_textlen == 0) {
                entry_length = start_tag1_len + strlen(label);
                start_tag = (char *) start_tag1;
            } else {
                entry_length = start_tag2_len + strlen(label);
                start_tag = (char *) start_tag2;
            }
            entry_text = malloc(entry_length + 1);
            sprintf(entry_text, start_tag, label);
            if (entry_startpos < prev_startpos) {
                curr = first;
            }
            curr = mobi_list_insert(curr, SIZE_MAX,
                                    (unsigned char *) entry_text,
                                    entry_length, true, entry_startpos);
            prev_startpos = entry_startpos;
            if (curr == NULL) {
                mobi_list_del_all(first);
                return MOBI_MALLOC_FAILED;
            }
            new_size += curr->size;
            if (entry_textlen > 0) {
                curr = mobi_list_insert(curr, SIZE_MAX,
                                        (unsigned char *) end_tag,
                                        end_tag_len, false, entry_startpos + entry_textlen);
                if (curr == NULL) {
                    mobi_list_del_all(first);
                    return MOBI_MALLOC_FAILED;
                }
                new_size += curr->size;
            }
            i++;
        }
    }
    if (first && first->next) {
        /* save */
        unsigned char *new_data = malloc((size_t) new_size);
        unsigned char *data_out = new_data;
        MOBIFragment *fragdata = first;
        while (fragdata) {
            memcpy(data_out, fragdata->fragment, fragdata->size);
            data_out += fragdata->size;
            fragdata = mobi_list_del(fragdata);
        }
        free(part->data);
        part->data = new_data;
        part->size = (size_t) new_size;
    } else {
        mobi_list_del(first);
    }
    return MOBI_SUCCESS;
}

/**
 @brief Replace offset-links with html-links
 
 @param[in,out] rawml Structure rawml will be filled with reconstructed parts and resources
 @return MOBI_RET status code (on success MOBI_SUCCESS)
 */
MOBI_RET mobi_reconstruct_links(const MOBIRawml *rawml) {
    if (rawml == NULL) {
        debug_print("%s\n", "Rawml not initialized\n");
        return MOBI_INIT_FAILED;
    }
    MOBI_RET ret;
    if (rawml->version != MOBI_NOTSET && rawml->version >= 8) {
        /* kf8 gymnastics */
        ret = mobi_reconstruct_links_kf8(rawml);
    } else {
        /* kf7 format and older */
        ret = mobi_reconstruct_links_kf7(rawml);
    }
    return ret;
}

/**
 @brief Call callback function for each text record
 
 @param[in,out] rawml Structure rawml will be filled with reconstructed parts and resources
 @param[in,out] cb Callback function
 @return MOBI_RET status code (on success MOBI_SUCCESS)
 */
MOBI_RET mobi_iterate_txtparts(MOBIRawml *rawml, MOBI_RET (*cb) (MOBIPart *)) {
    MOBIPart *parts[] = {
        rawml->markup, /* html files */
        rawml->flow->next /* css, skip first unparsed html part */
    };
    size_t i;
    for (i = 0; i < 2; i++) {
        MOBIPart *part = parts[i];
        while (part) {
            if (part->type == T_HTML || part->type == T_CSS) {
                MOBI_RET ret = cb(part);
                if (ret != MOBI_SUCCESS) {
                    return ret;
                }
            }
            part = part->next;
        }
    }
    return MOBI_SUCCESS;
}

/**
 @brief Convert MOBIPart part data to utf8
 
 @param[in,out] part MOBIPart part
 @return MOBI_RET status code (on success MOBI_SUCCESS)
 */
MOBI_RET mobi_markup_to_utf8(MOBIPart *part) {
    if (part == NULL) {
        return MOBI_INIT_FAILED;
    }
    unsigned char *text = part->data;
    size_t length = part->size;
    /* extreme case in which each input character is converted
     to 3-byte utf-8 sequence */
    size_t out_length = 3 * length + 1;
    char *out_text = malloc(out_length);
    if (out_text == NULL) {
        debug_print("%s", "Memory allocation failed\n");
        return MOBI_MALLOC_FAILED;
    }
    MOBI_RET ret = mobi_cp1252_to_utf8(out_text, (const char *) text, &out_length, length);
    free(text);
    if (ret != MOBI_SUCCESS || out_length == 0) {
        debug_print("%s", "conversion from cp1252 to utf8 failed\n");
        free(out_text);
        return ret;
    }
    text = malloc(out_length);
    if (text == NULL) {
        debug_print("%s", "Memory allocation failed\n");
        free(out_text);
        return MOBI_MALLOC_FAILED;
    }
    memcpy(text, out_text, out_length);
    free(out_text);
    part->data = text;
    part->size = out_length;
    return MOBI_SUCCESS;
}

/**
 @brief Parse raw records into html flow parts, markup parts, resources and indices
 
 @param[in,out] rawml Structure rawml will be filled with reconstructed parts and resources
 @param[in] m MOBIData structure
 @return MOBI_RET status code (on success MOBI_SUCCESS)
 */
MOBI_RET mobi_parse_rawml(MOBIRawml *rawml, const MOBIData *m) {
    
    MOBI_RET ret;
    if (m == NULL) {
        debug_print("%s", "Mobi structure not initialized\n");
        return MOBI_INIT_FAILED;
    }
    if (rawml == NULL) {
        return MOBI_INIT_FAILED;
    }
    
    /* Get maximal size of text data */
    const size_t maxlen = mobi_get_text_maxsize(m);
    char *text = malloc(maxlen + 1);
    if (text == NULL) {
        debug_print("%s", "Memory allocation failed\n");
        return MOBI_MALLOC_FAILED;
    }
    /* Extract text records, unpack, merge and copy it to text string */
    size_t length = maxlen;
    ret = mobi_get_rawml(m, text, &length);
    if (ret != MOBI_SUCCESS) {
        debug_print("%s", "Error parsing text\n");
        free(text);
        return MOBI_MALLOC_FAILED;
    }
    
    if (mobi_exists_fdst(m)) {
        /* Skip parsing if section count less or equal than 1 */
        if (m->mh->fdst_section_count && *m->mh->fdst_section_count > 1) {
            ret = mobi_parse_fdst(m, rawml);
            if (ret != MOBI_SUCCESS) {
                free(text);
                return ret;
            }
        }
    }
    ret = mobi_reconstruct_flow(rawml, text, length);
    free(text);
    if (ret != MOBI_SUCCESS) {
        return ret;
    }
    ret = mobi_reconstruct_resources(m, rawml);
    if (ret != MOBI_SUCCESS) {
        return ret;
    }
    const size_t offset = mobi_get_kf8offset(m);
    /* skeleton index */
    if (mobi_exists_skel_indx(m) && mobi_exists_frag_indx(m)) {
        const size_t indx_record_number = *m->mh->skeleton_index + offset;
        /* to be freed in mobi_free_rawml */
        MOBIIndx *skel_meta = mobi_init_indx();
        ret = mobi_parse_index(m, skel_meta, indx_record_number);
        if (ret != MOBI_SUCCESS) {
            return ret;
        }
        rawml->skel = skel_meta;
    }
    
    /* fragment index */
    if (mobi_exists_frag_indx(m)) {
        MOBIIndx *frag_meta = mobi_init_indx();
        const size_t indx_record_number = *m->mh->fragment_index + offset;
        ret = mobi_parse_index(m, frag_meta, indx_record_number);
        if (ret != MOBI_SUCCESS) {
            return ret;
        }
        rawml->frag = frag_meta;
    }
    
    /* guide index */
    if (mobi_exists_guide_indx(m)) {
        MOBIIndx *guide_meta = mobi_init_indx();
        const size_t indx_record_number = *m->mh->guide_index + offset;
        ret = mobi_parse_index(m, guide_meta, indx_record_number);
        if (ret != MOBI_SUCCESS) {
            return ret;
        }
        rawml->guide = guide_meta;
    }
    
    /* ncx index */
    if (mobi_exists_ncx(m)) {
        MOBIIndx *ncx_meta = mobi_init_indx();
        const size_t indx_record_number = *m->mh->ncx_index + offset;
        ret = mobi_parse_index(m, ncx_meta, indx_record_number);
        if (ret != MOBI_SUCCESS) {
            return ret;
        }
        rawml->ncx = ncx_meta;
    }
    
    /* orth index */
    /* FIXME: works only for old non-KF8 formats */
    if (rawml->version < 8 && mobi_exists_orth(m)) {
        MOBIIndx *orth_meta = mobi_init_indx();
        const size_t indx_record_number = *m->mh->orth_index + offset;
        ret = mobi_parse_index(m, orth_meta, indx_record_number);
        if (ret != MOBI_SUCCESS) {
            return ret;
        }
        rawml->orth = orth_meta;
    }
    
    ret = mobi_reconstruct_parts(rawml);
    if (ret != MOBI_SUCCESS) {
        return ret;
    }
#ifdef USE_LIBXML2
    ret = mobi_build_opf(rawml, m);
    if (ret != MOBI_SUCCESS) {
        return ret;
    }
#endif
    ret = mobi_reconstruct_links(rawml);
    if (ret != MOBI_SUCCESS) {
        return ret;
    }
    if (mobi_is_cp1252(m)) {
        ret = mobi_iterate_txtparts(rawml, mobi_markup_to_utf8);
        if (ret != MOBI_SUCCESS) {
            return ret;
        }
    }
    return MOBI_SUCCESS;
}


