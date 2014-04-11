//
//  util.c
//  mobi
//
//  Created by Bartek on 08.04.14.
//  Copyright (c) 2014 Bartek. All rights reserved.
//

#include "util.h"

void mobi_get_fullname(MOBIData *m, char *fullname, size_t len) {
    fullname[0] = '\0';
    if (m == NULL) {
        printf("Mobi structure not initialized\n");
        return;
    }
    MOBIPdbRecord *record0 = mobi_get_record_by_seqnumber(m, 0);
    if (m->mh == NULL || m->mh->full_name_offset == NULL || record0 == NULL) {
        return;
    }
    strncpy(fullname, record0->data + *m->mh->full_name_offset, len);
}

MOBIPdbRecord * mobi_get_record_by_uid(MOBIData *m, size_t uid) {
    MOBIPdbRecord *curr;
    if (m == NULL) {
        printf("Mobi structure not initialized\n");
        return NULL;
    }
    if (m->rec == NULL) {
        return NULL;
    }
    curr = m->rec;
    while (curr != NULL) {
        if (curr->uid == uid) {
            return curr;
        }
        curr = curr->next;
    }
    return NULL;
}

MOBIPdbRecord * mobi_get_record_by_seqnumber(MOBIData *m, size_t num) {
    MOBIPdbRecord *curr;
    if (m == NULL) {
        printf("Mobi structure not initialized\n");
        return NULL;
    }
    if (m->rec == NULL) {
        return NULL;
    }
    int i = 0;
    curr = m->rec;
    while (curr != NULL) {
        if (i++ == num) {
            return curr;
        }
        curr = curr->next;
    }
    return NULL;
}

int mobi_delete_record_by_seqnumber(MOBIData *m, size_t num) {
    MOBIPdbRecord *curr, *prev;
    if (m == NULL) {
        printf("Mobi structure not initialized\n");
        return MOBI_ERROR;
    }
    if (m->rec == NULL) {
        return MOBI_ERROR;
    }
    int i = 0;
    curr = m->rec;
    prev = NULL;
    while (curr != NULL) {
        if (i++ == num) {
            if (prev == NULL) {
                m->rec = curr->next;
            } else {
                prev->next = curr->next;
            }
            free(curr->data);
            curr->data = NULL;
            free(curr);
            curr = NULL;
            return MOBI_SUCCESS;
        }
        prev = curr;
        curr = curr->next;
    }
    return MOBI_SUCCESS;
}

MOBIExtHeader * mobi_get_exthtag_by_uid(MOBIData *m, size_t uid) {
    MOBIExtHeader *curr;
    if (m == NULL) {
        printf("Mobi structure not initialized\n");
        return NULL;
    }
    if (m->eh == NULL) {
        return NULL;
    }
    curr = m->eh;
    while (curr != NULL) {
        if (curr->uid == uid) {
            return curr;
        }
        curr = curr->next;
    }
    return NULL;
}

size_t sizeof_trailing_entry(MOBIPdbRecord *record, size_t psize) {
    size_t bitpos = 0;
    size_t result = 0;
    uint8_t v;
    while (1) {
        v = *(record->data + psize - 1);
        result |= (v & 0x7F) << bitpos;
        bitpos += 7;
        psize -= 1;
        if ((v & 0x80) != 0 || (bitpos >= 28) || (psize == 0)) {
            return result;
        }
    }
}

size_t mobi_get_record_extrasize(MOBIPdbRecord *record, uint16_t flags) {
    size_t num, size;
    num = 0;
    size = record->size;
    int mb_flag = flags & 1;
    flags >>= 1;
    while (flags) {
        if (flags & 1) {
            num += sizeof_trailing_entry(record, size - num);
        }
        flags >>= 1;
        }
        if (mb_flag){
            num += (*(record->data + size - num - 1) & 0x3) + 1;
        }
    return num;
}

/*size_t mobi_get_record_extrasize(MOBIPdbRecord *record, uint16_t flags) {
    size_t extra_size = 0, offset = 1;
    uint8_t b;
    for (int bit = 15; bit > 0; bit--) {
        if (flags & (1 << bit)) {
            // bit is set
            int bit_count = 0;
            do {
                // read at most 4 * 7-bit ints, bit 7 set stops search
                b = *(record->data + record->size - offset);
                extra_size |= (b & 0x7f) << bit_count;
                bit_count += 7;
                offset++;
            } while (!(b & 0x80) && (bit_count < 28) && offset < record->size);
            offset += extra_size - 1;
        }
    };
    // check bit 0
    if (flags & 1) {
        if (offset < record->size) {
            b = *(record->data + record->size - offset);
            // two first bits hold size
            extra_size += (b & 0x3) + 1;
        }
        
    }
    return extra_size;
}*/

// wrapper for mobi_get_rawml and mobi_dump_rawml
int mobi_decompress_content(MOBIData *m, char *text, FILE *file, size_t len, int dump) {
    MOBIPdbRecord *curr;
    size_t text_rec_index;
    size_t offset = 0;
    size_t text_length = 0;
    if (m == NULL) {
        printf("Mobi structure not initialized\n");
        return MOBI_ERROR;
    }
    // check if we want to parse kf8 part of joint file
    if (m->use_kf8 && m->next != NULL) {
        int kf8_offset = mobi_get_kf8boundary(m->next);
        if (kf8_offset >= 0) {
            // kf8 boundary + 1 * record0
            offset = kf8_offset + 1;
        }
    }
    if (m->rh == NULL || m->rh->text_record_count == 0) {
        printf("Text records not found in MOBI header\n");
        return MOBI_ERROR;
    }
    text_rec_index = 1 + offset;
    size_t text_rec_count = m->rh->text_record_count;
    uint16_t compression_type = m->rh->compression_type;
    // check for extra data at the end of text files
    uint16_t extra_flags = 0, extra_size = 0;
    if (m->mh && m->mh->extra_flags) {
        extra_flags = *m->mh->extra_flags;
    }
    // get first text record
    curr = mobi_get_record_by_seqnumber(m, text_rec_index);
    
    size_t d_size, record_size;
    char decompressed[2*RECORD0_RECORD_SIZE_MAX + 32]; // FIXME debug
    MOBIHuffCdic *huffcdic = NULL;
    if (compression_type == RECORD0_HUFF_COMPRESSION) {
        // load huff/cdic tables
        huffcdic = mobi_init_huffcdic(m);
    }
    // get following CDIC records
    while (text_rec_count--) {
        if (curr->uid == 17622) { // FIXME debug
            ;;
        }
        if (extra_flags) {
            extra_size = mobi_get_record_extrasize(curr, extra_flags);
        }
        record_size = curr->size - extra_size;
        switch (compression_type) {
            case RECORD0_NO_COMPRESSION:
                // no compression
                strncat(decompressed, curr->data, curr->size);
                d_size = curr->size;
                break;
            case RECORD0_PALMDOC_COMPRESSION:
                // palmdoc lz77 compression
                d_size = mobi_decompress_lz77(decompressed, curr->data, record_size);
                break;
            case RECORD0_HUFF_COMPRESSION:
                // mobi huffman compression
                d_size = mobi_decompress_huffman(decompressed, curr->data, record_size, huffcdic, 0);
                if (d_size > RECORD0_RECORD_SIZE_MAX) {
                    d_size = RECORD0_RECORD_SIZE_MAX;
                }
                break;
            default:
                printf("Unknown compression type\n");
                return MOBI_ERROR;
        }
        curr = curr->next;
        text_length += d_size;

        if (dump) {
            fwrite(decompressed, 1, d_size, file);
        } else {
            if (text_length > len) {
                printf("Text buffer too small\n");
                // free huff/cdic tables
                if (compression_type == RECORD0_HUFF_COMPRESSION) {
                    mobi_free_huffcdic(huffcdic);
                }
                return MOBI_ERROR;
            }
            strncat(text, decompressed, d_size);
        }
    }
    // free huff/cdic tables
    if (compression_type == RECORD0_HUFF_COMPRESSION) {
        mobi_free_huffcdic(huffcdic);
    }
    return MOBI_SUCCESS;
}

// copy raw text to text buffer
int mobi_get_rawml(MOBIData *m, char *text, size_t len) {
    if (m->rh->text_length > len) {
        printf("Text buffer smaller then text size declared in record0 header\n");
        return MOBI_ERROR;
    }
    text[0] = '\0';
    int ret = mobi_decompress_content(m, text, NULL, len, 0);
    return ret;
}

// dump raw text records to open file descriptor
int mobi_dump_rawml(MOBIData *m, FILE *file) {
    int ret = mobi_decompress_content(m, NULL, file, 0, 1);
    return ret;
}

// return kf8 boundary record sequential number or -1 if no such record
int mobi_get_kf8boundary(MOBIData *m) {
    MOBIExtHeader *exth_tag;
    MOBIPdbRecord *record;
    uint32_t rec_number;
    if (m == NULL) {
        printf("Mobi structure not initialized\n");
        return -1;
    }
    exth_tag = mobi_get_exthtag_by_uid(m, MOBI_EXTH_KF8BOUNDARY);
    if (exth_tag != NULL) {
        rec_number = * (uint32_t*) exth_tag->data;
        if (is_littleendian()) {
            rec_number = endian_swap32(rec_number);
        }
        rec_number--;
        record = mobi_get_record_by_seqnumber(m, rec_number);
        if (record) {
            if(strcmp(record->data, "BOUNDARY") == 0) {
                return rec_number;
            }
        }
    }
    return -1;
}

int mobi_swap_mobidata(MOBIData *m) {
    MOBIData *tmp;
    tmp = malloc(sizeof(MOBIData));
    if (tmp == NULL) {
        printf("memory allocation failed while swaping data\n");
        return MOBI_ERROR;
    }
    tmp->rh = m->rh;
    tmp->mh = m->mh;
    tmp->eh = m->eh;
    m->rh = m->next->rh;
    m->mh = m->next->mh;
    m->eh = m->next->eh;
    m->next->rh = tmp->rh;
    m->next->mh = tmp->mh;
    m->next->eh = tmp->eh;
    free(tmp);
    tmp = NULL;
    return MOBI_SUCCESS;
}
