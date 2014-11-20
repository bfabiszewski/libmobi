# Libmobi

C library for handling Mobipocket (MOBI) ebook format documents.
It is in a beta stage currently marked as version 0.1.

There is a simple program included in the project: mobitool.c.
It may serve as an example how to use the library.

## What works:
- reading and parsing: 
  - some older text Palmdoc formats, 
  - Mobipocket files, 
  - newer MOBI files including KF8 format,
  - Replica Print files
- recreating source files using indices
- reconstructing references (links and embedded) in html files
- reconstructing source structure that can be fed back to kindlegen
- reconstructing dictionary markup (orth, infl tags)
- handling encrypted documents

## Todo:
- process RESC records
- exporting to EPUB documents
- writing MOBI documents

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
    $ sudo make install

## Usage
- single include file: `#include <mobi.h>`
- linker flag: `-lmobi`

## Requirements
- compiler supporting C99
- tested with gcc (>=4.2.4), clang (llvm >=3.4)
- builds on Linux, MacOS X, Windows (MinGW)
- works cross-compiled on Kindle :)
- zlib (optional, configure --with-zlib=no to use included miniz.c instead)
- libxml2 (optional, enables OPF handling, configure --with-libxml2=no to disable)

## License:
- LGPL, either version 3, or any later

## Credits:
- The huffman decompression and KF8 parsing algorithms were learned by studying python source code of [KindleUnpack](http://wiki.mobileread.com/wiki/KindleUnpack) distributed with following license:

        Based on initial mobipocket version Copyright © 2009 Charles M. Hannum 
        Extensive Extensions and Improvements Copyright © 2009-2014 
        By P. Durrant, K. Hendricks, S. Siebert, fandrieu, DiapDealer, nickredding, tkeo.
        This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, version 3.
    
- Thanks to all contributors of Mobileread [MOBI wiki](http://wiki.mobileread.com/wiki/MOBI)
