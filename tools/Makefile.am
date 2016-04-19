# tools

# this lists the binaries to produce, the (non-PHONY, binary) targets in
# the previous manual Makefile
AM_CPPFLAGS = -I$(top_builddir)/src

bin_PROGRAMS = mobitool mobimeta
man_MANS = mobitool.1 mobimeta.1

mobitool_SOURCES = mobitool.c common.c
mobitool_DEPENDENCIES = $(top_builddir)/src/libmobi.la
mobitool_LDADD = $(top_builddir)/src/libmobi.la
mobitool_CFLAGS = $(ISO99_SOURCE) $(DEBUG_CFLAGS) -D_POSIX_C_SOURCE=200112L
mobitool_LDFLAGS = $(TOOLS_STATIC)

mobimeta_SOURCES = mobimeta.c common.c
mobimeta_DEPENDENCIES = $(top_builddir)/src/libmobi.la
mobimeta_LDADD = $(top_builddir)/src/libmobi.la
mobimeta_CFLAGS = $(ISO99_SOURCE) $(DEBUG_CFLAGS) -D_POSIX_C_SOURCE=200112L
mobimeta_LDFLAGS = $(TOOLS_STATIC)
