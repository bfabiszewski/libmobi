## mobitool
    usage: mobitool [-deimrsuvx7] [-o dir] [-p pid] [-P serial] filename
        without arguments prints document metadata and exits
        -d         dump rawml text record
        -e         create EPUB file (with -s will dump EPUB source)
        -i         print detailed metadata
        -m         print records metadata
        -o dir     save output to dir folder
        -p pid     set pid for decryption
        -P serial  set device serial number for decryption
        -r         dump raw records
        -s         dump recreated source files
        -u         show rusage
        -v         show version and exit
        -x         extract conversion source and log (if present)
        -7         parse KF7 part of hybrid file (by default KF8 part is parsed)

## mobimeta
    usage: mobimeta [-a | -s meta=value[,meta=value,...]] [-d meta[,meta,...]] [-p pid] [-P serial] [-v] filein [fileout]
        without arguments prints document metadata and exits
        -a meta=value  add metadata
        -d meta        delete metadata
        -s meta=value  set metadata
        -p pid         set pid for decryption
        -P serial      set device serial number for decryption
        -v             show version and exit