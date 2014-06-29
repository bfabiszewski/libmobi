# tools

# this lists the binaries to produce, the (non-PHONY, binary) targets in
# the previous manual Makefile
bin_PROGRAMS = mobitool
man_MANS = mobitool.1
AM_CPPFLAGS = -I$(top_builddir)/src
mobitool_SOURCES = mobitool.c
mobitool_DEPENDENCIES = $(top_builddir)/src/libmobi.la
mobitool_LDADD = $(top_builddir)/src/libmobi.la
mobitool_CFLAGS = $(ISO99_SOURCE) $(DEBUG_CFLAGS) -D_POSIX_C_SOURCE=200112L
mobitool_LDFLAGS = $(MOBITOOL_STATIC)
