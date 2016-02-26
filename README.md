# Libmobi

C library for handling Mobipocket/Kindle (MOBI) ebook format documents.
Current version supports reading and parsing functions.

There is a simple program included in the project: mobitool.c.
It may serve as an example how to use the library.

## What works:
- reading and parsing: 
  - some older text Palmdoc formats (pdb), 
  - Mobipocket files (prc, mobi), 
  - newer MOBI files including KF8 format (azw, azw3),
  - Replica Print files (azw4)
- recreating source files using indices
- reconstructing references (links and embedded) in html files
- reconstructing source structure that can be fed back to kindlegen
- reconstructing dictionary markup (orth, infl tags)
- handling encrypted documents

## Todo:
- writing MOBI documents
- process RESC records
- exporting to EPUB documents

## Doxygen documentation:
- [functions](http://www.fabiszewski.net/libmobi/group__mobi__export.html),
- [structures for the raw, unparsed records metadata and data](http://www.fabiszewski.net/libmobi/group__raw__structs.html),
- [structures for the parsed records metadata and data](http://www.fabiszewski.net/libmobi/group__parsed__structs.html),
- [enums](http://www.fabiszewski.net/libmobi/group__mobi__enums.html)

## Source:
- [on github](https://github.com/bfabiszewski/libmobi/)

## Installation:

    $ ./autogen.sh
    $ ./configure
    $ make
    [optionally] $ make test
    $ sudo make install

## Optionally provided Xcode and MSVC++ project files

## Usage
- single include file: `#include <mobi.h>`
- linker flag: `-lmobi`

## Requirements
- compiler supporting C99
- zlib (optional, configure --with-zlib=no to use included miniz.c instead)
- libxml2 (optional, enables OPF handling, configure --with-libxml2=no to disable)
- tested with gcc (>=4.2.4), clang (llvm >=3.4), sun c (>=5.13), MSVC++ (2015)
- builds on Linux, MacOS X, Windows (MSVC++, MinGW), Solaris
- tested architectures: x86, x86-64, arm, ppc
- works cross-compiled on Kindle :)

## Tests
- [![Travis status](https://travis-ci.org/bfabiszewski/libmobi.svg?branch=public)](https://travis-ci.org/bfabiszewski/libmobi)
- [![Coverity status](https://scan.coverity.com/projects/3521/badge.svg)](https://scan.coverity.com/projects/3521)

## License:
- LGPL, either version 3, or any later

## Credits:
- The huffman decompression and KF8 parsing algorithms were learned by studying python source code of [KindleUnpack](http://wiki.mobileread.com/wiki/KindleUnpack) distributed with following license:

        Based on initial mobipocket version Copyright © 2009 Charles M. Hannum 
        Extensive Extensions and Improvements Copyright © 2009-2014 
        By P. Durrant, K. Hendricks, S. Siebert, fandrieu, DiapDealer, nickredding, tkeo.
        This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, version 3.
    
- Thanks to all contributors of Mobileread [MOBI wiki](http://wiki.mobileread.com/wiki/MOBI)
