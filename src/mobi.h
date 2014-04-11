//
//  mobi.h
//  libmobi
//
//  Created by Bartek on 24.03.14.
//  Copyright (c) 2014 Bartek. All rights reserved.
//

#ifndef libmobi_mobi_h
#define libmobi_mobi_h

#include <stdint.h>
#include "buffer.h"
#include "compression.h"
#include "debug.h"

#define MOBI_ERROR -1
#define MOBI_SUCCESS 0

#define MOBI_USE_KF8 1
#define MOBI_USE_KF7 0

#define EPOCH_MAC_DIFF   2082844800
#define PALMDB_HEADER_LEN 78
#define PALMDB_NAME_SIZE_MAX    32
#define PALMDB_ATTRIBUTE_DEFAULT    0
#define PALMDB_VERSION_DEFAULT  0
#define PALMDB_MODNUM_DEFAULT   0
#define PALMDB_APPINFO_DEFAULT  0
#define PALMDB_SORTINFO_DEFAULT 0
#define PALMDB_TYPE_DEFAULT "BOOK"
#define PALMDB_CREATOR_DEFAULT  "MOBI"
#define PALMDB_NEXTREC_DEFAULT  0

#define RECORD0_HEADER_LEN 16
#define RECORD0_NO_COMPRESSION 1
#define RECORD0_PALMDOC_COMPRESSION 2
#define RECORD0_HUFF_COMPRESSION 17480
#define RECORD0_RECORD_SIZE_MAX 4096
#define RECORD0_NO_ENCRYPTION 0
#define RECORD0_OLD_ENCRYPTION 1
#define RECORD0_MOBI_ENCRYPTION 2

#define PDB_RECORD_INFO_SIZE 8

#define MOBI_MAGIC "MOBI"
#define EXTH_MAGIC "EXTH"
#define HUFF_MAGIC "HUFF"
#define CDIC_MAGIC "CDIC"

#define CDIC_HEADER_LEN 16
#define HUFF_HEADER_LEN 24
#define HUFF_RECORD_MINSIZE 2584

// EXTH
#define DRM_SERVER_ID 1
#define DRM_COMMERCE_ID 2
#define DRM_EBOOKBASE_BOOK_ID 3

#define MOBI_EXTH_AUTHOR 100 // <dc:Creator>
#define MOBI_EXTH_PUBLISHER 101 // <dc:Publisher>
#define MOBI_EXTH_IMPRINT 102 // <Imprint>
#define MOBI_EXTH_DESCRIPTION 103 // <dc:Description>
#define MOBI_EXTH_ISBN 104 // <dc:Identifier scheme="ISBN">
#define MOBI_EXTH_SUBJECT 105 // <dc:Subject>
#define MOBI_EXTH_PUBLISHINGDATE 106 // <dc:Date>
#define MOBI_EXTH_REVIEW 107 // <Review>
#define MOBI_EXTH_CONTRIBUTOR 108 // <dc:Contributor>
#define MOBI_EXTH_RIGHTS 109 // <dc:Rights>
#define MOBI_EXTH_SUBJECTCODE 110 // <dc:Subject BASICCode="subjectcode">
#define MOBI_EXTH_TYPE 111 // <dc:Type>
#define MOBI_EXTH_SOURCE 112 // <dc:Source>
#define MOBI_EXTH_ASIN 113
#define MOBI_EXTH_VERSION 114
#define MOBI_EXTH_SAMPLE 115
#define MOBI_EXTH_STARTREADING 116
#define MOBI_EXTH_ADULT 117 // <Adult>
#define MOBI_EXTH_PRICE 118 // <SRP>
#define MOBI_EXTH_PRICECURRENCY 119 // <SRP Currency="currency">
#define MOBI_EXTH_KF8BOUNDARY 121
#define MOBI_EXTH_COUNTRESOURCES 125
#define MOBI_EXTH_KF8OVERURI 129

#define MOBI_EXTH_DICTNAME 200 // <DictionaryVeryShortName>
#define MOBI_EXTH_COVEROFFSET 201 // <EmbeddedCover>
#define MOBI_EXTH_THUMBOFFSET 202
#define MOBI_EXTH_HASFAKECOVER 203
#define MOBI_EXTH_CREATORSOFT 204
#define MOBI_EXTH_CREATORMAJOR 205
#define MOBI_EXTH_CREATORMINOR 206
#define MOBI_EXTH_CREATORBUILD 207
#define MOBI_EXTH_WATERMARK 208
#define MOBI_EXTH_TAMPERKEYS 209

#define MOBI_EXTH_FONTSIGNATURE 300

#define MOBI_EXTH_CLIPPINGLIMIT 401
#define MOBI_EXTH_PUBLISHERLIMIT 402
#define MOBI_EXTH_TTS 404
#define MOBI_EXTH_RENAL 405
#define MOBI_EXTH_RENALEXPIRE 406

#define MOBI_EXTH_CDETYPE 501
#define MOBI_EXTH_LASTUPDATE 502
#define MOBI_EXTH_UPDATEDTITLE 503
#define MOBI_EXTH_LANGUAGE 524 // <dc:language>
#define MOBI_EXTH_ALIGNMENT 525
#define MOBI_EXTH_CREATORBUILD2 535





typedef struct  {
    char name[PALMDB_NAME_SIZE_MAX + 1]; // zero terminated, trimmed title+author
    uint16_t attributes; // PALMDB_ATTRIBUTE_DEFAULT
    uint16_t version; // PALMDB_VERSION_DEFAULT
    uint32_t ctime; // creation time
    uint32_t mtime; // modification time
    uint32_t btime; // backup time
    uint32_t mod_num; // PALMDB_MODNUM_DEFAULT
    uint32_t appinfo_offset; // PALMDB_APPINFO_DEFAULT
    uint32_t sortinfo_offset; // PALMDB_SORTINFO_DEFAULT
    char type[5]; // PALMDB_TYPE_DEFAULT
    char creator[5]; // PALMDB_CREATOR_DEFAULT
    uint32_t uid; // used internally to identify record
    uint32_t next_rec; // PALMDB_NEXTREC_DEFAULT
    uint16_t rec_count; // number of records in the file 
} MOBIPdbHeader;



typedef struct pdb_record {
    size_t offset;
    size_t size;
    uint8_t attributes;
    uint32_t uid;
    char *data;
    struct pdb_record *next;
} MOBIPdbRecord;

typedef struct exth {
    int uid;
    size_t size;
    void *data;
    struct exth *next;
} MOBIExtHeader;

typedef struct {
    // PalmDOC header (extended), offset 0, length 16
    uint16_t compression_type; // 0; 1 == no compression, 2 = PalmDOC compression, 17480 = HUFF/CDIC compression
    //uint16_t unused; // 2; 0
    uint32_t text_length; // 4; uncompressed length of the entire text of the book
    uint16_t text_record_count; // 8; number of PDB records used for the text of the book
    uint16_t text_record_size; // 10; maximum size of each record containing text, always 4096
    uint16_t encryption_type; // 12; 0 == no encryption, 1 = Old Mobipocket Encryption, 2 = Mobipocket Encryption
    uint16_t unknown1; // 14; usually 0
} MOBIRecord0Header;

typedef struct {
    // MOBI header, offset 16
    char mobi_magic[5]; // 16: M O B I { 77, 79, 66, 73 }
    uint32_t *header_length; // 20: the length of the MOBI header, including the previous 4 bytes
    uint32_t *mobi_type; // 24: mobipocket file type
    uint32_t *text_encoding; // 28: 1252 = CP1252, 65001 = UTF-8
    uint32_t *uid; // 32: unique id
    uint32_t *file_version; // 36: mobipocket format
    uint32_t *orth_index; // 40: section number of orthographic meta index. 0xFFFFFFFF if index is not available.
    uint32_t *infl_index; // 44: section number of inflection meta index. 0xFFFFFFFF if index is not available.
    uint32_t *names_index; // 48: section number of names meta index. 0xFFFFFFFF if index is not available.
    uint32_t *keys_index; // 52: section number of keys meta index. 0xFFFFFFFF if index is not available.
    uint32_t *extra0_index; // 56: section number of extra 0 meta index. 0xFFFFFFFF if index is not available.
    uint32_t *extra1_index; // 60: section number of extra 1 meta index. 0xFFFFFFFF if index is not available.
    uint32_t *extra2_index; // 64: section number of extra 2 meta index. 0xFFFFFFFF if index is not available.
    uint32_t *extra3_index; // 68: section number of extra 3 meta index. 0xFFFFFFFF if index is not available.
    uint32_t *extra4_index; // 72: section number of extra 4 meta index. 0xFFFFFFFF if index is not available.
    uint32_t *extra5_index; // 76: section number of extra 5 meta index. 0xFFFFFFFF if index is not available.
    uint32_t *non_text_index; // 80: first record number (starting with 0) that's not the book's text
    uint32_t *full_name_offset; // 84: offset in record 0 (not from start of file) of the full name of the book
    uint32_t *full_name_length; // 88:
    uint32_t *locale; // 92: low byte is main language 09= English, next byte is dialect, 08 = British, 04 = US
    uint32_t *input_lang; // 96: input language for a dictionary
    uint32_t *output_lang; // 100: output language for a dictionary
    uint32_t *min_version; // 104: minimum mobipocket version support needed to read this file.
    uint32_t *image_index; // 108: first record number (starting with 0) that contains an image (sequential)
    uint32_t *huff_rec_index; // 112: first huffman compression record.
    uint32_t *huff_rec_count; // 116:
    uint32_t *huff_table_offset; // 120:
    uint32_t *huff_table_length; // 124:
    uint32_t *exth_flags; // 128: bitfield. if bit 6 (0x40) is set, then there's an EXTH record
    // 32 unknown bytes 0?
    // unknown2
    // unknown3
    // unknown4
    // unknown5
    uint32_t *unknown6; // 164: use 0xFFFFFFFF
    uint32_t *drm_offset; // 168: offset to DRM key info in DRMed files. 0xFFFFFFFF if no DRM
    uint32_t *drm_count; // 172: number of entries in DRM info
    uint32_t *drm_size; // 176: number of bytes in DRM info
    uint32_t *drm_flags; // 180: some flags concerning the DRM info
    // 8 unknown bytes 0?
    // unknown7
    // unknown8
    uint16_t *first_text_index; // 192:
    uint16_t *last_text_index; // 194:
    uint32_t *unknown9; // 196:
    uint32_t *fcis_index; // 200:
    uint32_t *fcis_count; // 204:
    uint32_t *flis_index; // 208:
    uint32_t *flis_count; // 212:
    uint32_t *unknown10; // 216:
    uint32_t *unknown11; // 220:
    uint32_t *srcs_index; // 224:
    uint32_t *srcs_count; // 228:
    uint32_t *unknown12; // 232:
    uint32_t *unknown13; // 236:
    // uint16_t fill 0
    uint16_t *extra_flags; // 242:
    uint32_t *ncx_index; // 244:
    uint32_t *unknown14; // 248:
    uint32_t *unknown15; // 252:
    uint32_t *datp_index; // 256:
    uint32_t *unknown16; // 260:
    uint32_t *unknown17; // 264:
    uint32_t *unknown18; // 268:
    uint32_t *unknown19; // 272:
    uint32_t *unknown20; // 276:
} MOBIMobiHeader;

typedef struct m {
    uint8_t use_kf8;
    MOBIPdbHeader *ph;
    MOBIRecord0Header *rh;
    MOBIMobiHeader *mh;
    MOBIExtHeader *eh;
    MOBIPdbRecord *rec;
    struct m *next;
} MOBIData;

void write_mobi(void);
int mobi_load_file(MOBIData *m, FILE *file);
int mobi_load_filename(MOBIData *m, const char *path);
MOBIData * mobi_init();
void mobi_free(MOBIData *m);

int mobi_parse_huffdic(MOBIData *m, MOBIHuffCdic *cdic);
MOBIPdbRecord * mobi_get_record_by_uid(MOBIData *m, size_t uid);
MOBIPdbRecord * mobi_get_record_by_seqnumber(MOBIData *m, size_t uid);
int mobi_get_rawml(MOBIData *m, char *text, size_t len);
int mobi_dump_rawml(MOBIData *m, FILE *file);
void mobi_get_fullname(MOBIData *m, char *fullname, size_t len);
int mobi_get_kf8boundary(MOBIData *m);
#endif
