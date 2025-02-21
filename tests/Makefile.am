
CPPFLAGS  = -I$(top_srcdir)/include/trurl -I$(top_srcdir)/include
LDADD = $(top_builddir)/libtrurl.la @CHECK_LIBS@

EXTRA_DIST = test.h
#CFLAGS = @CHECK_CFLAGS@

check_PROGRAMS = test_stream test_iobuf test_hash test_list test_str test_array test_buf \
		 test_hash_func test_sort_func
TESTS = $(check_PROGRAMS)

#noinst_PROGRAMS = test_hash_func
#test_hash_func_LDADD = $(top_builddir)/libtrurl.la $(NULL)
