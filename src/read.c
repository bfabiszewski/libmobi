//
//  read.c
//  mobi
//
//  Created by Bartek on 26.03.14.
//  Copyright (c) 2014 Bartek. All rights reserved.
//

#include <stdio.h>
#include <string.h>
#include "read.h"

int mobi_load_pdbheader(MOBIData *m, FILE *file) {
    MOBIBuffer *buf;
    if (m == NULL) {
        printf("Mobi structure not initialized\n");
        return MOBI_ERROR;
    }
    if (!file) {
        return MOBI_ERROR;
    }
    buf = buffer_init(PALMDB_HEADER_LEN);
    if (buf == NULL) {
        return MOBI_ERROR;
    }
    size_t len = fread(buf->data, 1, PALMDB_HEADER_LEN, file);
    if (len != PALMDB_HEADER_LEN) {
        buffer_free(buf);
        return MOBI_ERROR;
    }
    m->ph = calloc(1, sizeof(MOBIPdbHeader));
    if (m->ph == NULL) {
        printf("Memory allocation for pdb header failed\n");
        return MOBI_ERROR;
    }
    // parse header
    buffer_getstring(m->ph->name, buf, PALMDB_NAME_SIZE_MAX);
    m->ph->name[PALMDB_NAME_SIZE_MAX] = '\0';
    m->ph->attributes = buffer_get16(buf);
    m->ph->version = buffer_get16(buf);
    m->ph->ctime = buffer_get32(buf);
    m->ph->mtime = buffer_get32(buf);
    m->ph->btime = buffer_get32(buf);
    m->ph->mod_num = buffer_get32(buf);
    m->ph->appinfo_offset = buffer_get32(buf);
    m->ph->sortinfo_offset = buffer_get32(buf);
    buffer_getstring(m->ph->type, buf, 4);
    m->ph->type[4] = '\0';
    buffer_getstring(m->ph->creator, buf, 4);
    m->ph->creator[4] = '\0';
    m->ph->uid = buffer_get32(buf);
    m->ph->next_rec = buffer_get32(buf);
    m->ph->rec_count = buffer_get16(buf);
    buffer_free(buf);
    return MOBI_SUCCESS;
}

int mobi_load_reclist(MOBIData *m, FILE *file) {
    if (m == NULL) {
        printf("Mobi structure not initialized\n");
        return MOBI_ERROR;
    }
    if (!file) {
        printf("File not ready\n");
        return MOBI_ERROR;
    }
    int i;
    MOBIBuffer *buf;
    MOBIPdbRecord *curr;
    m->rec = calloc(1, sizeof(MOBIPdbRecord));
    if (m->rec == NULL) {
        printf("Memory allocation for pdb record failed\n");
        return MOBI_ERROR;
    }
    curr = m->rec;
    for (i = 0; i < m->ph->rec_count; i++) {
        buf = buffer_init(PDB_RECORD_INFO_SIZE);
        if (buf == NULL) {
            return MOBI_ERROR;
        }
        size_t len = fread(buf->data, 1, PDB_RECORD_INFO_SIZE, file);
        if (len != PDB_RECORD_INFO_SIZE) {
            buffer_free(buf);
            return MOBI_ERROR;
        }
        if (i > 0) {
            curr->next = calloc(1, sizeof(MOBIPdbRecord));
            if (curr->next == NULL) {
                printf("Memory allocation for pdb record failed\n");
                return MOBI_ERROR;
            }
            curr = curr->next;
        }
        curr->offset = buffer_get32(buf);
        curr->attributes = buffer_get8(buf);
        uint8_t h = buffer_get8(buf);
        uint16_t l = buffer_get16(buf);
        curr->uid =  h << 16 | l;
        curr->next = NULL;
        buffer_free(buf);
    }
    return MOBI_SUCCESS;
}

int mobi_load_recdata(MOBIData *m, FILE *file) {
    MOBIPdbRecord *curr, *next;
    int ret;
    if (m == NULL) {
        printf("Mobi structure not initialized\n");
        return MOBI_ERROR;
    }
    curr = m->rec;
    while (curr != NULL) {
        size_t size;
        if (curr->next != NULL) {
            next = curr->next;
            size = next->offset - curr->offset;
        } else {
            fseek(file, 0, SEEK_END);
            size = ftell(file) - curr->offset;
            next = NULL;
        }

        curr->size = size;
        ret = mobi_load_rec(curr, file);
        if (ret == MOBI_ERROR) {
            printf("Error loading record uid %i data\n", curr->uid);
            mobi_free_rec(m);
            return MOBI_ERROR;
        }
        curr = next;
    }
    return MOBI_SUCCESS;
}

int mobi_load_rec(MOBIPdbRecord *rec, FILE *file) {
    size_t len;
    int ret;
    ret = fseek(file, rec->offset, SEEK_SET);
    if (ret != 0) {
        printf("Record %i not found\n", rec->uid);
        return MOBI_ERROR;
    }
    rec->data = malloc(rec->size);
    if (rec->data == NULL) {
        printf("Memory allocation for pdb record data failed\n");
        return MOBI_ERROR;
    }
    len = fread(rec->data, 1, rec->size, file);
    if (len < rec->size) {
        printf("Truncated data in record %i\n", rec->uid);
        rec->size = len;
        char *ptr = realloc(rec->data, len);
        if (ptr) {
            rec->data = ptr;
        }
    }
    return MOBI_SUCCESS;
}

int mobi_parse_extheader(MOBIData *m, MOBIBuffer *buf) {
    size_t saved_maxlen;
    char exth_magic[4];
    size_t exth_length;
    size_t rec_count;
    if (m == NULL) {
        printf("Mobi structure not initialized\n");
        return MOBI_ERROR;
    }
    buffer_getstring(exth_magic, buf, 4);
    exth_length = buffer_get32(buf);
    rec_count = buffer_get32(buf);
    if (strncmp(exth_magic, EXTH_MAGIC, 4) != 0 ||
        exth_length + buf->offset + 8 > buf->maxlen ||
        rec_count == 0) {
        return MOBI_ERROR;
    }
    saved_maxlen = buf->maxlen;
    buf->maxlen = exth_length + buf->offset - 8;
    m->eh = calloc(1, sizeof(MOBIExtHeader));
    if (m->eh == NULL) {
        printf("Memory allocation for EXTH header failed\n");
        return MOBI_ERROR;
    }
    int i;
    MOBIExtHeader *curr;
    curr = m->eh;
    for (i = 0; i < rec_count; i++) {
        if (i > 0) {
            curr->next = calloc(1, sizeof(MOBIExtHeader));
            if (curr->next == NULL) {
                printf("Memory allocation for EXTH header failed\n");
                return MOBI_ERROR;
            }
            curr = curr->next;
        }
        curr->uid = buffer_get32(buf);
        // data size = record size minus 8 bytes for uid and size
        curr->size = buffer_get32(buf) - 8;
        if (curr->size == 0) {
            printf("Skip record %i, data too short\n", curr->uid);
            continue;
        }
        curr->data = malloc(curr->size);
        if (curr->data == NULL) {
            printf("Memory allocation for EXTH record %i failed\n", curr->uid);
            mobi_free_eh(m);
            return MOBI_ERROR;
        }
        buffer_getraw(curr->data, buf, curr->size);
        curr->next = NULL;
    }    
    buf->maxlen = saved_maxlen;
    return MOBI_SUCCESS;
}

int mobi_parse_mobiheader(MOBIData *m, MOBIBuffer *buf) {
    size_t saved_maxlen;
    if (m == NULL) {
        printf("Mobi structure not initialized\n");
        return MOBI_ERROR;
    }
    m->mh = calloc(1, sizeof(MOBIMobiHeader));
    if (m->mh == NULL) {
        printf("Memory allocation for MOBI header failed\n");
        return MOBI_ERROR;
    }
    buffer_getstring(m->mh->mobi_magic, buf, 4);
    m->mh->mobi_magic[4] = '\0';
    buffer_copy32(&m->mh->header_length, buf);
    if (strcmp(m->mh->mobi_magic, MOBI_MAGIC) != 0 || m->mh->header_length == NULL) {
        printf("MOBI header not found\n");
        mobi_free_mh(m);
        return MOBI_ERROR;
    }
    saved_maxlen = buf->maxlen;
    // read only declared MOBI header length (curr offset minus 8 already read bytes)
    buf->maxlen = *m->mh->header_length + buf->offset - 8;
    buffer_copy32(&m->mh->mobi_type, buf);
    buffer_copy32(&m->mh->text_encoding, buf);
    buffer_copy32(&m->mh->uid, buf);
    buffer_copy32(&m->mh->file_version, buf);
    buffer_copy32(&m->mh->orth_index, buf);
    buffer_copy32(&m->mh->infl_index, buf);
    buffer_copy32(&m->mh->names_index, buf);
    buffer_copy32(&m->mh->keys_index, buf);
    buffer_copy32(&m->mh->extra0_index, buf);
    buffer_copy32(&m->mh->extra1_index, buf);
    buffer_copy32(&m->mh->extra2_index, buf);
    buffer_copy32(&m->mh->extra3_index, buf);
    buffer_copy32(&m->mh->extra4_index, buf);
    buffer_copy32(&m->mh->extra5_index, buf);
    buffer_copy32(&m->mh->non_text_index, buf);
    buffer_copy32(&m->mh->full_name_offset, buf);
    buffer_copy32(&m->mh->full_name_length, buf);
    buffer_copy32(&m->mh->locale, buf);
    buffer_copy32(&m->mh->input_lang, buf);
    buffer_copy32(&m->mh->output_lang, buf);
    buffer_copy32(&m->mh->min_version, buf);
    buffer_copy32(&m->mh->image_index, buf);
    buffer_copy32(&m->mh->huff_rec_index, buf);
    buffer_copy32(&m->mh->huff_rec_count, buf);
    buffer_copy32(&m->mh->huff_table_offset, buf);
    buffer_copy32(&m->mh->huff_table_length, buf);
    buffer_copy32(&m->mh->exth_flags, buf);
    buf->offset += 32; // 32 unknown bytes
    buffer_copy32(&m->mh->unknown6, buf);
    buffer_copy32(&m->mh->drm_offset, buf);
    buffer_copy32(&m->mh->drm_count, buf);
    buffer_copy32(&m->mh->drm_size, buf);
    buffer_copy32(&m->mh->drm_flags, buf);
    buf->offset += 8; // 8 unknown bytes
    buffer_copy16(&m->mh->first_text_index, buf);
    buffer_copy16(&m->mh->last_text_index, buf);
    buffer_copy32(&m->mh->unknown9, buf);
    buffer_copy32(&m->mh->fcis_index, buf);
    buffer_copy32(&m->mh->fcis_count, buf);
    buffer_copy32(&m->mh->flis_index, buf);
    buffer_copy32(&m->mh->flis_count, buf);
    buffer_copy32(&m->mh->unknown10, buf);
    buffer_copy32(&m->mh->unknown11, buf);
    buffer_copy32(&m->mh->srcs_index, buf);
    buffer_copy32(&m->mh->srcs_count, buf);
    buffer_copy32(&m->mh->unknown12, buf);
    buffer_copy32(&m->mh->unknown13, buf);
    buf->offset += 2; // 2 byte fill
    buffer_copy16(&m->mh->extra_flags, buf);
    buffer_copy32(&m->mh->ncx_index, buf);
    buffer_copy32(&m->mh->unknown14, buf);
    buffer_copy32(&m->mh->unknown15, buf);
    buffer_copy32(&m->mh->datp_index, buf);
    buffer_copy32(&m->mh->unknown16, buf);
    buffer_copy32(&m->mh->unknown17, buf);
    buffer_copy32(&m->mh->unknown18, buf);
    buffer_copy32(&m->mh->unknown19, buf);
    buffer_copy32(&m->mh->unknown20, buf);
    if (buf->maxlen > buf->offset) {
        buf->offset = buf->maxlen;
    }
    buf->maxlen = saved_maxlen;
    return MOBI_SUCCESS;
}


// parse
int mobi_parse_record0(MOBIData *m, size_t seqnumber) {
    MOBIBuffer *buf;
    MOBIPdbRecord *record0;
    if (m == NULL) {
        printf("Mobi structure not initialized\n");
        return MOBI_ERROR;
    }
    record0 = mobi_get_record_by_seqnumber(m, seqnumber);
    if (record0 == NULL || record0->size == 0) {
        printf("Record 0 not loaded\n");
        return MOBI_ERROR;
    }
    buf = buffer_init(record0->size);
    if (buf == NULL) {
        return MOBI_ERROR;
    }
    memcpy(buf->data, record0->data, record0->size);
    m->rh = calloc(1, sizeof(MOBIRecord0Header));
    if (m->rh == NULL) {
        printf("Memory allocation for record 0 header failed\n");
        return MOBI_ERROR;
    }
    // parse palmdoc header
    m->rh->compression_type = buffer_get16(buf);
    buf->offset += 2; // unused, 0
    m->rh->text_length = buffer_get32(buf);
    m->rh->text_record_count = buffer_get16(buf);
    m->rh->text_record_size = buffer_get16(buf);
    m->rh->encryption_type = buffer_get16(buf);
    m->rh->unknown1 = buffer_get16(buf);
    if (strcmp(m->ph->type, "BOOK") == 0 && strcmp(m->ph->creator, "MOBI") == 0) {
        // parse mobi header 
        mobi_parse_mobiheader(m, buf);
        // parse exth header
        mobi_parse_extheader(m, buf);
    } 
    buffer_free(buf);
    return MOBI_SUCCESS;
}

int mobi_parse_huff(MOBIHuffCdic *huffcdic, MOBIPdbRecord *record) {
    MOBIBuffer *buf;
    char huff_magic[5];
    size_t header_length;
    buf = buffer_init(record->size);
    if (buf == NULL) {
        return MOBI_ERROR;
    }
    memcpy(buf->data, record->data, record->size);
    buffer_getstring(huff_magic, buf, 4);
    header_length = buffer_get32(buf);
    if (strncmp(huff_magic, HUFF_MAGIC, 4) != 0 || header_length < HUFF_HEADER_LEN) {
        printf("HUFF wrong magic: %s\n", huff_magic);
        buffer_free(buf);
        return MOBI_ERROR;
    }
    size_t data1_offset = buffer_get32(buf);
    size_t data2_offset = buffer_get32(buf);
    // skip little-endian table offsets
    buf->offset = data1_offset;
    if (buf->offset + (256 * 4) > buf->maxlen) {
        printf("HUFF data1 too short\n");
        buffer_free(buf);
        return MOBI_ERROR;
    }
    // read 256 indices from data1 big-endian
    for (int i = 0; i < 256; i++) {
        huffcdic->table1[i] = buffer_get32(buf);
    }
    buf->offset = data2_offset;
    if (buf->offset + (64 * 4) > buf->maxlen) {
        printf("HUFF data2 too short\n");
        buffer_free(buf);
        return MOBI_ERROR;
    }
    // read 32 mincode-maxcode pairs from data2 big-endian
    uint32_t mincode, maxcode;
    huffcdic->mincode_table[0] = 0;
    huffcdic->maxcode_table[0] = 0xFFFFFFFF;
    for (int i = 1; i < 33; i++) {
        mincode = buffer_get32(buf);
        maxcode = buffer_get32(buf);
        huffcdic->mincode_table[i] =  mincode << (32 - i);
        huffcdic->maxcode_table[i] =  ((maxcode + 1) << (32 - i)) - 1;
    }
    buffer_free(buf);
    return MOBI_SUCCESS;
}

int mobi_parse_cdic(MOBIHuffCdic *huffcdic, MOBIPdbRecord *record, int num) {
    MOBIBuffer *buf;
    char cdic_magic[5];
    size_t header_length, index_count, code_length;
    buf = buffer_init(record->size);
    if (buf == NULL) {
        return MOBI_ERROR;
    }
    memcpy(buf->data, record->data, record->size);
    buffer_getstring(cdic_magic, buf, 4);
    header_length = buffer_get32(buf);
    if (strncmp(cdic_magic, CDIC_MAGIC, 4) != 0 || header_length < CDIC_HEADER_LEN) {
        printf("CDIC wrong magic: %s\n", cdic_magic);
        buffer_free(buf);
        return MOBI_ERROR;
    }
    // variables in huffcdic initialized to zero with calloc
    // save initial count and length
    index_count = buffer_get32(buf);
    code_length = buffer_get32(buf);
    if (huffcdic->code_length && huffcdic->code_length != code_length) {
        printf("Warning: CDIC different code length %zu in record %i, previous was %zu\n", huffcdic->code_length, record->uid, code_length);
    }
    if (huffcdic->index_count && huffcdic->index_count != index_count) {
        printf("Warning: CDIC different index count %zu in record %i, previous was %zu\n", huffcdic->index_count, record->uid, index_count);
    }
    huffcdic->code_length = code_length;
    huffcdic->index_count = index_count;
    if (index_count == 0) {
        printf("CDIC index count is null");
        buffer_free(buf);
        return MOBI_ERROR;
    }
    // allocate memory for symbol offsets if not already allocated
    if (num == 0) {
        huffcdic->symbol_offsets = malloc(index_count * sizeof(*huffcdic->symbol_offsets));
        if (huffcdic->symbol_offsets == NULL) {
            printf("CDIC cannot allocate memory");
            buffer_free(buf);
            return MOBI_ERROR;
        }
    }
    index_count -= huffcdic->index_read;
    // limit number of records read to code_length bits
    if (index_count >> code_length) {
        index_count = (1 << code_length);
    }
    if (buf->offset + (index_count * 2) > buf->maxlen) {
        printf("CDIC indices data too short\n");
        buffer_free(buf);
        free(huffcdic->symbol_offsets);
        return MOBI_ERROR;
    }
    // read i * 2 byte big-endian indices
    while (index_count--) {
        huffcdic->symbol_offsets[huffcdic->index_read++] = buffer_get16(buf);
    }
    if (buf->offset + code_length > buf->maxlen) {
        printf("CDIC dictionary data too short");
        free(huffcdic->symbol_offsets);
        buffer_free(buf);
        return MOBI_ERROR;
    }
    // copy pointer to data
    huffcdic->symbols[num] = record->data + CDIC_HEADER_LEN;
    // free buffer
    buffer_free(buf);
    return MOBI_SUCCESS;
}

int mobi_parse_huffdic(MOBIData *m, MOBIHuffCdic *huffcdic) {
    MOBIPdbRecord *curr;
    int ret, i = 0;
    if (m->mh == NULL || m->mh->huff_rec_index == NULL) {
        printf("HUFF/CDIC records metadata not found in MOBI header\n");
        return MOBI_ERROR;
    }
    size_t huff_rec_index = *m->mh->huff_rec_index;
    size_t huff_rec_count = *m->mh->huff_rec_count;
    curr = mobi_get_record_by_seqnumber(m, huff_rec_index);
    if (curr == NULL) {
        printf("HUFF record not found\n");
        return MOBI_ERROR;
    }
    if (curr->size < HUFF_RECORD_MINSIZE) {
        printf("HUFF record too short (%zu b)\n", curr->size);
        return MOBI_ERROR;
    }
    ret = mobi_parse_huff(huffcdic, curr);
    if (ret == MOBI_ERROR) {
        printf("HUFF parsing failed\n");
        return MOBI_ERROR;
    }
    //huff_rec_index++;
    curr = curr->next;
    // allocate memory for symbols data in each CDIC record
    huffcdic->symbols = malloc((huff_rec_count - 1) * sizeof(*huffcdic->symbols));
    // get following CDIC records
    while (i < huff_rec_count - 1) {
        ret = mobi_parse_cdic(huffcdic, curr, i++);
        if (ret == MOBI_ERROR) {
            printf("CDIC parsing failed\n");
            free(huffcdic->symbols);
            return MOBI_ERROR;
        }
        curr = curr->next;
    }
    
    return MOBI_SUCCESS;
}
int mobi_load_file(MOBIData *m, FILE *file) {
    int ret;
    if (m == NULL) {
        printf("Mobi structure not initialized\n");
        return MOBI_ERROR;
    }
    ret = mobi_load_pdbheader(m, file);

    if (strcmp(m->ph->type, "BOOK") != 0 && strcmp(m->ph->type, "TEXt") != 0) {
        printf("Unsupported file type: %s\n", m->ph->type);
        return MOBI_ERROR;
    }

    if (ret == MOBI_ERROR || m->ph->rec_count == 0) {
        printf("No records found\n");
        return MOBI_ERROR;
    }
    ret = mobi_load_reclist(m, file);
    if (ret == MOBI_ERROR) {
        return MOBI_ERROR;
    }
    ret = mobi_load_recdata(m, file);
    if (ret == MOBI_ERROR) {
        return MOBI_ERROR;
    }
    ret = mobi_parse_record0(m, 0);
    // if EXTH is loaded and use_kf8 flag is set parse KF8 record0 for joined mobi7/kf8 file
    if (m->eh && m->use_kf8) {
        int boundary_rec_number;
        boundary_rec_number = mobi_get_kf8boundary(m);
        if (boundary_rec_number >= 0) {
            // it is a joint mobi7/kf8 file
            m->next = mobi_init();
            // link pdb header and records data to kf8data structure
            m->next->ph = m->ph;
            m->next->rec = m->rec;
            // close next loop
            m->next->next = m;
            ret = mobi_parse_record0(m->next, boundary_rec_number + 1);
            mobi_swap_mobidata(m);
            }
    }
    return ret;
}

int mobi_load_filename(MOBIData *m, const char *path) {
    FILE *file;
    int ret;
    file = fopen(path, "rb");
    ret = mobi_load_file(m, file);
    fclose(file);
    return ret;
}
