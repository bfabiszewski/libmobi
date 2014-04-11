//
//  test.c
//  mobi
//
//  Created by Bartek on 25.03.14.
//  Copyright (c) 2014 Bartek. All rights reserved.
//

#include <stdio.h>
#include "../src/mobi.h"
//#include <mobi.h>

// FIXME: testing
#define DUMP_REC_OPT 0;
#define LOADFILENAME 1

void print_meta(MOBIData *m) {
    if (m->mh && m->mh->full_name_offset && m->mh->full_name_length) {
        char *full_name;
        size_t len = *m->mh->full_name_length;
        full_name = malloc(len + 1);
        mobi_get_fullname(m, full_name, len);
        printf("full name: %s\n", full_name);
        free(full_name);
    }
    printf("name: %s\n", m->ph->name);
    printf("attributes: %hu\n", m->ph->attributes);
    printf("version: %hu\n", m->ph->version);
    printf("ctime: %u\n", m->ph->ctime);
    printf("mtime: %u\n", m->ph->mtime);
    printf("mtime: %u\n", m->ph->mtime);
    printf("btime: %u\n", m->ph->btime);
    printf("mod_num: %u\n", m->ph->mod_num);
    printf("appinfo_offset: %u\n", m->ph->appinfo_offset);
    printf("sortinfo_offset: %u\n", m->ph->sortinfo_offset);
    printf("type: %s\n", m->ph->type);
    printf("creator: %s\n", m->ph->creator);
    printf("uid: %u\n", m->ph->uid);
    printf("next_rec: %u\n", m->ph->next_rec);
    printf("rec_count: %u\n", m->ph->rec_count);
    if (m->rh) {
        printf("\nRecord 0:\n");
        printf("compresion type: %u\n", m->rh->compression_type);
        printf("text length: %u\n", m->rh->text_length);
        printf("record count: %u\n", m->rh->text_record_count);
        printf("record size: %u\n", m->rh->text_record_size);
        printf("encryption type: %u\n", m->rh->encryption_type);
        printf("unknown: %u\n", m->rh->unknown1);
    }
    if (m->mh) {
        printf("identifier: %s\n", m->mh->mobi_magic);
        if(m->mh->header_length) { printf("header length: %u\n", *m->mh->header_length); }
        if(m->mh->mobi_type) { printf("mobi type: %u\n", *m->mh->mobi_type); }
        if(m->mh->text_encoding) { printf("text encoding: %u\n", *m->mh->text_encoding); }
        if(m->mh->uid) { printf("unique id: %u\n", *m->mh->uid); }
        if(m->mh->file_version) { printf("file version: %u\n", *m->mh->file_version); }
        if(m->mh->orth_index) { printf("orth index: %u\n", *m->mh->orth_index); }
        if(m->mh->infl_index) { printf("infl index: %u\n", *m->mh->infl_index); }
        if(m->mh->names_index) { printf("names index: %u\n", *m->mh->names_index); }
        if(m->mh->keys_index) { printf("keys index: %u\n", *m->mh->keys_index); }
        if(m->mh->extra0_index) { printf("extra0 index: %u\n", *m->mh->extra0_index); }
        if(m->mh->extra1_index) { printf("extra1 index: %u\n", *m->mh->extra1_index); }
        if(m->mh->extra2_index) { printf("extra2 index: %u\n", *m->mh->extra2_index); }
        if(m->mh->extra3_index) { printf("extra3 index: %u\n", *m->mh->extra3_index); }
        if(m->mh->extra4_index) { printf("extra4 index: %u\n", *m->mh->extra4_index); }
        if(m->mh->extra5_index) { printf("extra5 index: %u\n", *m->mh->extra5_index); }
        if(m->mh->non_text_index) { printf("non text index: %u\n", *m->mh->non_text_index); }
        if(m->mh->full_name_offset) { printf("full name offset: %u\n", *m->mh->full_name_offset); }
        if(m->mh->full_name_length) { printf("full name length: %u\n", *m->mh->full_name_length); }
        if(m->mh->locale) { printf("locale: %u\n", *m->mh->locale); }
        if(m->mh->input_lang) { printf("input lang: %u\n", *m->mh->input_lang); }
        if(m->mh->output_lang) { printf("outpu lang: %u\n", *m->mh->output_lang); }
        if(m->mh->min_version) { printf("minimal version: %u\n", *m->mh->min_version); }
        if(m->mh->image_index) { printf("first image index: %u\n", *m->mh->image_index); }
        if(m->mh->huff_rec_index) { printf("huffman record offset: %u\n", *m->mh->huff_rec_index); }
        if(m->mh->huff_rec_count) { printf("huffman record count: %u\n", *m->mh->huff_rec_count); }
        if(m->mh->huff_table_offset) { printf("huffman table offset: %u\n", *m->mh->huff_table_offset); }
        if(m->mh->huff_table_length) { printf("huffman table length: %u\n", *m->mh->huff_table_length); }
        if(m->mh->exth_flags) { printf("EXTH flags: %u\n", *m->mh->exth_flags); }
        if(m->mh->unknown6) { printf("unknown: %u\n", *m->mh->unknown6); }
        if(m->mh->drm_offset) { printf("drm offset: %u\n", *m->mh->drm_offset); }
        if(m->mh->drm_size) { printf("drm size: %u\n", *m->mh->drm_size); }
        if(m->mh->drm_flags) { printf("drm flags: %u\n", *m->mh->drm_flags); }
        if(m->mh->first_text_index) { printf("first text index: %u\n", *m->mh->first_text_index); }
        if(m->mh->last_text_index) { printf("last text index: %u\n", *m->mh->last_text_index); }
        if(m->mh->unknown9) { printf("unknown: %u\n", *m->mh->unknown9); }
        if(m->mh->fcis_index) { printf("FCIS index: %u\n", *m->mh->fcis_index); }
        if(m->mh->fcis_count) { printf("FCIS count: %u\n", *m->mh->fcis_count); }
        if(m->mh->flis_index) { printf("FLIS index: %u\n", *m->mh->flis_index); }
        if(m->mh->flis_count) { printf("FLIS count: %u\n", *m->mh->flis_count); }
        if(m->mh->unknown10) { printf("unknown: %u\n", *m->mh->unknown10); }
        if(m->mh->unknown11) { printf("unknown: %u\n", *m->mh->unknown11); }
        if(m->mh->srcs_index) { printf("SRCS index: %u\n", *m->mh->srcs_index); }
        if(m->mh->srcs_count) { printf("SRCS count: %u\n", *m->mh->srcs_count); }
        if(m->mh->unknown12) { printf("unknown: %u\n", *m->mh->unknown12); }
        if(m->mh->unknown13) { printf("unknown: %u\n", *m->mh->unknown13); }
        if(m->mh->extra_flags) { printf("extra record flags: %u\n", *m->mh->extra_flags); }
        if(m->mh->ncx_index) { printf("NCX offset: %u\n", *m->mh->ncx_index); }
        if(m->mh->unknown14) { printf("unknown: %u\n", *m->mh->unknown14); }
        if(m->mh->unknown15) { printf("unknown: %u\n", *m->mh->unknown15); }
        if(m->mh->datp_index) { printf("DATP index: %u\n", *m->mh->datp_index); }
        if(m->mh->unknown16) { printf("unknown: %u\n", *m->mh->unknown16); }
        if(m->mh->unknown17) { printf("unknown: %u\n", *m->mh->unknown17); }
        if(m->mh->unknown18) { printf("unknown: %u\n", *m->mh->unknown18); }
        if(m->mh->unknown19) { printf("unknown: %u\n", *m->mh->unknown19); }
        if(m->mh->unknown20) { printf("unknown: %u\n", *m->mh->unknown20); }
    }
}

void print_exth(MOBIData *m) {
    MOBIExtHeader *curr;
    if (m->eh == NULL) {
        return;
    }
    curr = m->eh;
    while (curr != NULL) {
        char *str;
        uint32_t val;
        str = calloc(1, curr->size+1);
        strncpy(str, curr->data, curr->size);
        val = *(uint32_t*) curr->data;
        if (is_littleendian()) {
            val = endian_swap32(val);
        }
        printf("id: %i\tval: %s (%u)\tsize: %zu\n", curr->uid, str, val, curr->size);
        free(str);
        curr = curr->next;
    }
}

void print_records_meta(MOBIData *m) {
    MOBIPdbRecord *currec;
    currec = m->rec;
    while (currec != NULL) {
        printf("offset: %zu\n", currec->offset);
        printf("size: %zu\n", currec->size);
        printf("attributes: %hhu\n", currec->attributes);
        printf("uid: %u\n", currec->uid);
        printf("\n");
        currec = currec->next;
    }
}

void dump_records(MOBIData *m, char *filepath) {
    MOBIPdbRecord *currec;
    FILE *file;
    char name[FILENAME_MAX];
    int i = 0;
    currec = m->rec;
    while (currec != NULL) {
        sprintf(name, "%spart_%i_uid_%i", filepath, i++, currec->uid);
        file = fopen(name, "wb");
        fwrite(currec->data, 1, currec->size, file);
        fclose(file);
        currec = currec->next;
    }
}

int dump_rawml(MOBIData *m, char *filepath) {
    FILE *file;
    int ret;
    char name[FILENAME_MAX];
    sprintf(name, "%srawml", filepath);
    file = fopen(name, "wb");
    ret = mobi_dump_rawml(m, file);
    fclose(file);
    return ret;
}

int loadfilename(const char *filename) {
    MOBIData *m;
    int ret = 0;
    m = mobi_init();
    if (m == NULL) {
        printf("init failed\n");
        return 1;
    }
    char filepath[FILENAME_MAX];
    char *p = strrchr(filename, '/');
    if (p) {
        p += 1;
        strncpy(filepath, filename, (p - filename));
        filepath[p - filename] = '\0';
    }
    else {
        filepath[0] = '\0';
    }
    m->use_kf8 = MOBI_USE_KF7;
    ret = mobi_load_filename(m, filename);
    print_meta(m);
    if (ret == MOBI_ERROR) {
        mobi_free(m);
        return 1;
    }
    print_exth(m);
    print_records_meta(m);
    int dump_rec_opt = DUMP_REC_OPT;
    if (dump_rec_opt) {
        dump_records(m, filepath);
    }
    ret = dump_rawml(m, filepath);
    mobi_free(m);
    return ret;
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        printf("usage: %s filename\n", argv[0]);
        return 1;
    }
    int command = LOADFILENAME;
    int ret = 0;
    char filename[FILENAME_MAX];
    strncpy(filename, argv[1], FILENAME_MAX - 1);
    switch (command) {
        case LOADFILENAME:
            ret = loadfilename(filename);
            break;
    }
    return ret;
}
