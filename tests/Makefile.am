
CPPFLAGS  = -I$(top_srcdir)/include/trurl -I$(top_srcdir)/include
LDADD = $(top_builddir)/libtrurl.la @CHECK_LIBS@

EXTRA_DIST = test.h
#CFLAGS = @CHECK_CFLAGS@

check_PROGRAMS = test_stream test_iobuf test_hash test_list test_str test_array test_buf
TESTS = $(check_PROGRAMS)
