# tools

# what flags you want to pass to the C compiler & linker
AM_CFLAGS = --pedantic -Wall -std=gnu99 -O2
AM_LDFLAGS =

# this lists the binaries to produce, the (non-PHONY, binary) targets in
# the previous manual Makefile
bin_PROGRAMS = mobitool
man_MANS = mobitool.1
mobitool_SOURCES = mobitool.c
mobitool_LDADD =  $(top_builddir)/src/libmobi.la
