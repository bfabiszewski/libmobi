# libmobi 

# what flags you want to pass to the C compiler & linker
AM_CFLAGS = --pedantic -Wall -std=gnu99 -O2
AM_LDFLAGS =

# this lists the binaries to produce, the (non-PHONY, binary) targets in
# the previous manual Makefile
lib_LTLIBRARIES = libmobi.la
libmobi_la_SOURCES = buffer.c compression.c debug.c memory.c read.c util.c write.c  \
                  buffer.h compression.h debug.h memory.h mobi.h read.h util.h write.h
