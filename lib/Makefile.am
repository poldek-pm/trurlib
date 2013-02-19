## Process this file with automake to create Makefile.in

noinst_LTLIBRARIES = libsys.la
if ENABLE_SELF_OBSTACK
OBSTACK_C = obstack.c
endif

libsys_la_SOURCES = $(OBSTACK_C)

noinst_HEADERS = obstack.h

AM_CPPFLAGS = -I$(top_srcdir)
AM_CFLAGS = @AM_CFLAGS@

EXTRA_DIST = obstack.c

