# tools

# this lists the binaries to produce, the (non-PHONY, binary) targets in
# the previous manual Makefile
AM_CPPFLAGS = -I$(top_builddir)/src

bin_PROGRAMS = mobitool mobimeta
man_MANS = mobitool.1 mobimeta.1

noinst_LIBRARIES = libcommon.a
libcommon_a_SOURCES = common.c common.h

mobitool_SOURCES = mobitool.c mobitool.h
mobitool_DEPENDENCIES = $(top_builddir)/src/libmobi.la libcommon.a
mobitool_LDADD = $(top_builddir)/src/libmobi.la libcommon.a
mobitool_CFLAGS = $(ISO99_SOURCE) $(DEBUG_CFLAGS) -D_POSIX_C_SOURCE=200112L
mobitool_LDFLAGS = $(TOOLS_STATIC)

if USE_XMLWRITER
# miniz.c zip functions are needed for epub creation
noinst_LIBRARIES += libminiz.a
libminiz_a_SOURCES = ../src/miniz.c ../src/miniz.h
libminiz_a_CFLAGS = $(MINIZ_CFLAGS) -DMINIZ_NO_ZLIB_COMPATIBLE_NAMES
mobitool_DEPENDENCIES += libminiz.a
mobitool_LDADD += libminiz.a
endif

mobimeta_SOURCES = mobimeta.c mobimeta.h
mobimeta_DEPENDENCIES = $(top_builddir)/src/libmobi.la libcommon.a
mobimeta_LDADD = $(top_builddir)/src/libmobi.la libcommon.a
mobimeta_CFLAGS = $(ISO99_SOURCE) $(DEBUG_CFLAGS) -D_POSIX_C_SOURCE=200112L
mobimeta_LDFLAGS = $(TOOLS_STATIC)
