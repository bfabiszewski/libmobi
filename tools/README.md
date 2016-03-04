    usage: mobitool [-demrsuvx7] [-o dir] [-p pid] filename
        without arguments prints document metadata and exits
        -d      dump rawml text record
        -e      create EPUB file (with -s will dump EPUB source)
        -m      print records metadata
        -o dir  save output to dir folder
        -p pid  set pid for decryption
        -r      dump raw records
        -s      dump recreated source files
        -u      show rusage
        -v      show version and exit
        -x      extract conversion source and log (if present)
        -7      parse KF7 part of hybrid file (by default KF8 part is parsed)