# libmobi 

lib_LTLIBRARIES = libmobi.la
libmobi_la_SOURCES = buffer.c compression.c debug.c index.c memory.c parse_rawml.c read.c structure.c util.c write.c  \
                  buffer.h compression.h config.h debug.h index.h memory.h mobi.h parse_rawml.h read.h structure.h util.h write.h
if USE_LIBXML2
libmobi_la_SOURCES += opf.c opf.h
endif
if USE_MINIZ
libmobi_la_SOURCES += miniz.c miniz.h
endif
if USE_ENCRYPTION
libmobi_la_SOURCES += encryption.c encryption.h
endif
include_HEADERS = mobi.h
libmobi_la_LDFLAGS = $(AVOID_VERSION) $(NO_UNDEFINED) $(DARWIN_LDFLAGS) $(LIBZ_LDFLAGS) $(LIBXML2_LDFLAGS)
libmobi_la_CFLAGS = $(VISIBILITY_HIDDEN) $(ISO99_SOURCE) $(DEBUG_CFLAGS) $(MINIZ_CFLAGS) $(LIBXML2_CFLAGS)
