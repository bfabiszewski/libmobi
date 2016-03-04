/*
 * Copyright (c) 2014 Bartek Fabiszewski
 * http://www.fabiszewski.net
 *
 * This file is part of libmobi.
 * Licensed under LGPL, either version 3, or any later.
 * See <http://www.gnu.org/licenses/>
 */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>

#include "write.h"
#include "util.h"
#include "debug.h"


/* 
   I focus my development on reading functions.
   Below are just a bunch of a quick tests,
   which will probably disappear or be thoroughly modified
   when I get to coding true writing routines.
*/

/* FIXME test */
/* buffer should be passed to this func */
MOBIBuffer * serialize_palmdb_header(void) {
    MOBIBuffer *buf;
    const char title[PALMDB_NAME_SIZE_MAX] = "TITLE";
    const size_t len = strlen(title);
    
    const uint32_t curtime = (uint32_t) time(NULL);
    const uint32_t uid = 0xff;
    const uint32_t rec_count = 1;
    buf = buffer_init(PALMDB_HEADER_LEN);
    if (buf == NULL) {
        return NULL;
    }
    buffer_addstring(buf, title);
    buffer_addzeros(buf, PALMDB_NAME_SIZE_MAX - len);
    buffer_add16(buf, PALMDB_ATTRIBUTE_DEFAULT);
    buffer_add16(buf, PALMDB_VERSION_DEFAULT);
    buffer_add32(buf, curtime); /* ctime */
    buffer_add32(buf, curtime); /* mtime */
    buffer_add32(buf, 0); /* btime */
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

/* FIXME test */
MOBIBuffer * serialize_record0_header(void) {
    uint32_t text_length = 0;
    uint16_t record_count = 0;
    MOBIBuffer *buf = buffer_init(RECORD0_HEADER_LEN);
    if (buf == NULL) {
        return NULL;
    }
    buffer_add16(buf, RECORD0_NO_COMPRESSION);
    buffer_add16(buf, 0);
    buffer_add32(buf, text_length);
    buffer_add16(buf, record_count);
    buffer_add16(buf, RECORD0_TEXT_SIZE_MAX);
    buffer_add16(buf, RECORD0_NO_ENCRYPTION);
    buffer_add16(buf, 0);
    return buf;
}

void buffer_output(FILE *file, MOBIBuffer *buf) {
    if (file) {
        fwrite(buf->data, 1, buf->offset, file);
        debug_print("Buffer length %zu bytes\n", buf->offset);
    }
    buffer_free(buf);
}

/* FIXME test */
MOBIPdbRecord * build_pdbrecord(uint32_t offset) {
    MOBIPdbRecord *record = NULL;
    record = malloc(sizeof(MOBIPdbRecord));
    record->data = malloc(RECORD0_TEXT_SIZE_MAX);
    char test[] = "<html><body>test</body></html>";
    memcpy(record->data, test, strlen(test));
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
    MOBIBuffer *buf = buffer_init(8);
    if (buf == NULL) {
        return NULL;
    }
    buffer_add32(buf, rec->offset);
    /* skip attributes, always 0; */
    buffer_add32(buf, rec->uid);
    return buf;
}

MOBIBuffer * serialize_pdbrecord(MOBIPdbRecord *rec) {
    MOBIBuffer *buf = buffer_init(RECORD0_TEXT_SIZE_MAX);
    if (buf) {
        buffer_addraw(buf, rec->data, rec->size);
    }
    return buf;
}

MOBIBuffer * serialize_file_end(void) {
    const unsigned char end[4] = EOF_MAGIC;
    MOBIBuffer *buf = buffer_init(4);
    if (buf) {
        buffer_addraw(buf, end, 4);
    }
    return buf;
}


/* FIXME test */
void write_mobi(void) {
    FILE *file = fopen("test.mobi","wb");
    if (file == NULL) {
        debug_print("%s\n", "Could not open file for writing");
        return;
    }
    MOBIBuffer *buf = serialize_palmdb_header();
    debug_print("%s\n", "Writing palmdb header");
    buffer_output(file, buf);
    MOBIPdbRecord *rec = build_pdbrecord(PALMDB_HEADER_LEN + PALMDB_RECORD_INFO_SIZE + 2);
    buf = serialize_record_info(rec);
    buf->maxlen += 2;
    buffer_addzeros(buf, 2);
    debug_print("%s\n", "Writing record info + 2 zeros");
    buffer_output(file, buf);
    buf = serialize_record0_header();
    debug_print("%s\n", "Writing record0 header");
    buffer_output(file, buf);
    buf = serialize_pdbrecord(rec);
    /* TODO: improve freeing of rec buffer, see buffer_free */
    free(rec->data);
    free(rec);
    debug_print("%s\n", "Writing pdb record");
    buffer_output(file, buf);
    buf = serialize_file_end();
    debug_print("%s\n", "Writing 4 end chars");
    buffer_output(file, buf);
    
    fclose(file);
}
