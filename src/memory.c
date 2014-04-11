//
//  memory.c
//  mobi
//
//  Created by Bartek on 31.03.14.
//  Copyright (c) 2014 Bartek. All rights reserved.
//

#include <stdio.h>
#include "memory.h"

MOBIData * mobi_init() {
    MOBIData *m = NULL;
    m = calloc(1, sizeof(MOBIData));
	if (m == NULL) return NULL;
    m->use_kf8 = MOBI_USE_KF8;
    m->ph = NULL;
    m->rh = NULL;
    m->mh = NULL;
    m->eh = NULL;
    m->rec = NULL;
    m->next = NULL;
    return m;
}

void mobi_free_mh(MOBIData *m) {
    if (m->mh == NULL) {
        return;
    }
    free(m->mh->header_length);
    free(m->mh->mobi_type);
    free(m->mh->text_encoding);
    free(m->mh->uid);
    free(m->mh->file_version);
    free(m->mh->orth_index);
    free(m->mh->infl_index);
    free(m->mh->names_index);
    free(m->mh->keys_index);
    free(m->mh->extra0_index);
    free(m->mh->extra1_index);
    free(m->mh->extra2_index);
    free(m->mh->extra3_index);
    free(m->mh->extra4_index);
    free(m->mh->extra5_index);
    free(m->mh->non_text_index);
    free(m->mh->full_name_offset);
    free(m->mh->full_name_length);
    free(m->mh->locale);
    free(m->mh->input_lang);
    free(m->mh->output_lang);
    free(m->mh->min_version);
    free(m->mh->image_index);
    free(m->mh->huff_rec_index);
    free(m->mh->huff_rec_count);
    free(m->mh->huff_table_offset);
    free(m->mh->huff_table_length);
    free(m->mh->exth_flags);
    free(m->mh->unknown6);
    free(m->mh->drm_offset);
    free(m->mh->drm_count);
    free(m->mh->drm_size);
    free(m->mh->drm_flags);
    free(m->mh->first_text_index);
    free(m->mh->last_text_index);
    free(m->mh->unknown9);
    free(m->mh->fcis_index);
    free(m->mh->fcis_count);
    free(m->mh->flis_index);
    free(m->mh->flis_count);
    free(m->mh->unknown10);
    free(m->mh->unknown11);
    free(m->mh->srcs_index);
    free(m->mh->srcs_count);
    free(m->mh->unknown12);
    free(m->mh->unknown13);
    free(m->mh->extra_flags);
    free(m->mh->ncx_index);
    free(m->mh->unknown14);
    free(m->mh->unknown15);
    free(m->mh->datp_index);
    free(m->mh->unknown16);
    free(m->mh->unknown17);
    free(m->mh->unknown18);
    free(m->mh->unknown19);
    free(m->mh->unknown20);
    free(m->mh);
    m->mh = NULL;
}

void mobi_free_rec(MOBIData *m) {
    MOBIPdbRecord *curr, *tmp;
    curr = m->rec;
    while (curr != NULL) {
        tmp = curr;
        curr = curr->next;
        free(tmp->data);
        free(tmp);
        tmp = NULL;
    }
    m->rec = NULL;
}

void mobi_free_eh(MOBIData *m) {
    MOBIExtHeader *curr, *tmp;
    curr = m->eh;
    while (curr != NULL) {
        tmp = curr;
        curr = curr->next;
        free(tmp->data);
        free(tmp);
        tmp = NULL;
    }
    m->eh = NULL;
}

void mobi_free(MOBIData *m) {
    if (m == NULL) {
        return;
    }
    mobi_free_mh(m);
    mobi_free_eh(m);
    mobi_free_rec(m);
    free(m->ph);
    free(m->rh);
    if (m->next) {
        mobi_free_mh(m->next);
        mobi_free_eh(m->next);
        free(m->next->rh);
        free(m->next);
        m->next = NULL;
    }
    free(m);
    m = NULL;
}

MOBIHuffCdic * mobi_init_huffcdic(MOBIData *m) {
    MOBIHuffCdic *huffcdic;
    int ret;
    huffcdic = calloc(1, sizeof(MOBIHuffCdic));
    if (huffcdic == NULL) {
        printf("Memory allocation for huffcdic structure failed\n");
        return NULL;
    }
    ret = mobi_parse_huffdic(m, huffcdic);
    if (ret == MOBI_ERROR) {
        free(huffcdic);
        return NULL;
    }
    return huffcdic;
}

void mobi_free_huffcdic(MOBIHuffCdic *huffcdic) {
    free(huffcdic->symbol_offsets);
    free(huffcdic->symbols);
    free(huffcdic);
}
