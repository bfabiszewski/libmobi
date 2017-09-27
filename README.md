# Libmobi

C library for handling Mobipocket/Kindle (MOBI) ebook format documents.

For examples on how to use the library have a look at tools folder.

## Features:
- reading and parsing: 
  - some older text Palmdoc formats (pdb), 
  - Mobipocket files (prc, mobi), 
  - newer MOBI files including KF8 format (azw, azw3),
  - Replica Print files (azw4)
- recreating source files using indices
- reconstructing references (links and embedded) in html files
- reconstructing source structure that can be fed back to kindlegen
- reconstructing dictionary markup (orth, infl tags)
- writing back loaded documents
- metadata editing
- handling encrypted documents

## Todo:
- improve writing
- serialize rawml into raw records
- process RESC records

## Doxygen documentation:
- [functions](http://www.fabiszewski.net/libmobi/group__mobi__export.html),
- [structures for the raw, unparsed records metadata and data](http://www.fabiszewski.net/libmobi/group__raw__structs.html),
- [structures for the parsed records metadata and data](http://www.fabiszewski.net/libmobi/group__parsed__structs.html),
- [enums](http://www.fabiszewski.net/libmobi/group__mobi__enums.html)

## Source:
- [on github](https://github.com/bfabiszewski/libmobi/)

## Installation:

    [for git] $ ./autogen.sh
    $ ./configure
    $ make
    [optionally] $ make test
    $ sudo make install

## Optionally provided Xcode and MSVC++ project files

## Usage
- single include file: `#include <mobi.h>`
- linker flag: `-lmobi`
- basic usage:
```c
#include <mobi.h>

/* Initialize main MOBIData structure */
/* Must be deallocated with mobi_free() when not needed */
MOBIData *m = mobi_init();
if (m == NULL) { 
  return ERROR; 
}

/* Open file for reading */
FILE *file = fopen(fullpath, "rb");
if (file == NULL) {
  mobi_free(m);
  return ERROR;
}

/* Load file into MOBIData structure */
/* This structure will hold raw data/metadata from mobi document */
MOBI_RET mobi_ret = mobi_load_file(m, file);
fclose(file);
if (mobi_ret != MOBI_SUCCESS) { 
  mobi_free(m);
  return ERROR;
}

/* Initialize MOBIRawml structure */
/* Must be deallocated with mobi_free_rawml() when not needed */
/* In the next step this structure will be filled with parsed data */
MOBIRawml *rawml = mobi_init_rawml(m);
if (rawml == NULL) {
  mobi_free(m);
  return ERROR;
}
/* Raw data from MOBIData will be converted to html, css, fonts, media resources */
/* Parsed data will be available in MOBIRawml structure */
mobi_ret = mobi_parse_rawml(rawml, m);
if (mobi_ret != MOBI_SUCCESS) {
  mobi_free(m);
  mobi_free_rawml(rawml);
  return ERROR;
}

/* Do something useful here */
/* ... */
/* For examples how to access data in MOBIRawml structure see mobitool.c */

/* Free MOBIRawml structure */
mobi_free_rawml(rawml);

/* Free MOBIData structure */
mobi_free(m);

return SUCCESS;
```
- for examples of usage, see [tools](https://github.com/bfabiszewski/libmobi/tree/public/tools)


## Requirements
- compiler supporting C99
- zlib (optional, configure --with-zlib=no to use included miniz.c instead)
- libxml2 (optional, configure --with-libxml2=no to use internal xmlwriter)
- tested with gcc (>=4.2.4), clang (llvm >=3.4), sun c (>=5.13), MSVC++ (2015)
- builds on Linux, MacOS X, Windows (MSVC++, MinGW), Android, Solaris
- tested architectures: x86, x86-64, arm, ppc
- works cross-compiled on Kindle :)

## Tests
- [![Travis status](https://travis-ci.org/bfabiszewski/libmobi.svg?branch=public)](https://travis-ci.org/bfabiszewski/libmobi)
- [![Coverity status](https://scan.coverity.com/projects/3521/badge.svg)](https://scan.coverity.com/projects/3521)

## Projects using libmobi
- [KyBook 2 Reader](http://kybook-reader.com)
- [@Voice Aloud Reader](http://www.hyperionics.com/atVoice/)
- [QLMobi quicklook plugin](https://github.com/bfabiszewski/QLMobi/tree/master/QLMobi)
- [Librera Reader](http://librera.mobi)
- ... (let me know to include your project)

## License:
- LGPL, either version 3, or any later

## Credits:
- The huffman decompression and KF8 parsing algorithms were learned by studying python source code of [KindleUnpack](https://github.com/kevinhendricks/KindleUnpack).
- Thanks to all contributors of Mobileread [MOBI wiki](http://wiki.mobileread.com/wiki/MOBI)
