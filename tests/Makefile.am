TESTS = test_trurlib
CPPFLAGS  = -I$(top_srcdir)/include/trurl -I$(top_srcdir)/include

check_PROGRAMS = test_trurlib
test_trurlib_SOURCES = test.h test_trurlib.c test_niobuf.c test_nstream.c
test_trurlib_CFLAGS = @CHECK_CFLAGS@
test_trurlib_LDADD = $(top_builddir)/libtrurl.la @CHECK_LIBS@
