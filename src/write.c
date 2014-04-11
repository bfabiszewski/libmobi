//
//  write.c
//  mobi
//
//  Created by Bartek on 25.03.14.
//  Copyright (c) 2014 Bartek. All rights reserved.
//

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "write.h"

MOBIBuffer * serialize_palmdb_header(void) {
    MOBIBuffer *buf;
    size_t len;
    char title[PALMDB_NAME_SIZE_MAX];
    strcpy(title, "TITLE");
    len = strlen(title);
    
    uint32_t curtime = (uint32_t)(time(NULL) + EPOCH_MAC_DIFF);
    uint32_t uid = 0xff;
    uint32_t rec_count = 1;
    buf = buffer_init(PALMDB_HEADER_LEN);
    if (buf == NULL) {
        return NULL;
    }
    buffer_addstring(buf, title);
    buffer_addzeros(buf, PALMDB_NAME_SIZE_MAX - len);
    buffer_add16(buf, PALMDB_ATTRIBUTE_DEFAULT);
    buffer_add16(buf, PALMDB_VERSION_DEFAULT);
    buffer_add32(buf, curtime); // ctime
    buffer_add32(buf, curtime); // mtime
    buffer_add32(buf, 0); // btime
    buffer_add32(buf, PALMDB_MODNUM_DEFAULT);
    buffer_add32(buf, PALMDB_APPINFO_DEFAULT);
    buffer_add32(buf, PALMDB_SORTINFO_DEFAULT);
    buffer_addstring(buf, PALMDB_TYPE_DEFAULT);
    buffer_addstring(buf, PALMDB_CREATOR_DEFAULT);
    buffer_add32(buf, uid);
    buffer_add32(buf, PALMDB_NEXTREC_DEFAULT);
    buffer_add16(buf, rec_count);
    return buf;
}

MOBIBuffer * serialize_record0_header(void) {
    MOBIBuffer *buf;
    uint32_t text_length = 0;
    uint16_t record_count = 0;
    buf = buffer_init(RECORD0_HEADER_LEN);
    if (buf == NULL) {
        return NULL;
    }
    buffer_add16(buf, RECORD0_NO_COMPRESSION);
    buffer_add16(buf, 0);
    buffer_add32(buf, text_length);
    buffer_add16(buf, record_count);
    buffer_add16(buf, RECORD0_RECORD_SIZE_MAX);
    buffer_add16(buf, RECORD0_NO_ENCRYPTION);
    buffer_add16(buf, 0);
    return buf;
}

void buffer_output(FILE *file, MOBIBuffer *buf) {
    if (file) {
        fwrite(buf->data, 1, buf->offset, file);
        printf("Buffer length %zu bytes\n", buf->offset);
    }
    buffer_free(buf);
}

MOBIPdbRecord * build_pdbrecord(size_t offset) {
    MOBIPdbRecord *record = NULL;
    record = malloc(sizeof(MOBIPdbRecord));
    record->data = malloc(RECORD0_RECORD_SIZE_MAX);
    strncpy(record->data, "<html><body>test</body></html>", RECORD0_RECORD_SIZE_MAX);
    if (record->data == NULL) {
		free(record);
		return NULL;
	}
    record->offset = offset;
    record->size = offset;
    record->attributes = 0;
    record->uid = 0;
    return record;
}

MOBIBuffer * serialize_record_info(MOBIPdbRecord *rec) {
    MOBIBuffer *buf;
    buf = buffer_init(8);
    if (buf == NULL) {
        return NULL;
    }
    buffer_add32(buf, (uint32_t) rec->offset);
    //skip attributes, always 0;
    buffer_add32(buf, rec->uid);
    return buf;
}

MOBIBuffer * serialize_pdbrecord(MOBIPdbRecord *rec) {
    MOBIBuffer *buf;
    buf = buffer_init(RECORD0_RECORD_SIZE_MAX);
    if (buf) {
        buffer_addstring(buf, rec->data);
    }
    return buf;
}

MOBIBuffer * serialize_file_end(void) {
    MOBIBuffer *buf;
    char end[] = { 233, 142, 13, 10 };
    buf = buffer_init(4);
    if (buf) {
        buffer_addraw(buf, end, 4);
    }
    return buf;
}


void write_mobi(void) {
    FILE *file;
    MOBIBuffer *buf;
    MOBIPdbRecord *rec;
    file = fopen("/Users/baf/src/mobi_test/test.mobi","wb");
    buf = serialize_palmdb_header();
    printf("Writing palmdb header\n");
    buffer_output(file, buf);
    rec = build_pdbrecord(PALMDB_HEADER_LEN + PDB_RECORD_INFO_SIZE + 2);
    buf = serialize_record_info(rec);
    buf->maxlen += 2;
    buffer_addzeros(buf, 2);
    printf("Writing record info + 2 zeros\n");
    buffer_output(file, buf);
    buf = serialize_record0_header();
    printf("Writing record0 header\n");
    buffer_output(file, buf);
    buf = serialize_pdbrecord(rec);
    // TODO: improve freeing of rec buffer, see buffer_free
    free(rec->data);
    free(rec);
    printf("Writing pdb record\n");
    buffer_output(file, buf);
    buf = serialize_file_end();
    printf("Writing 4 end chars\n");
    buffer_output(file, buf);
    
    fclose(file);
}
