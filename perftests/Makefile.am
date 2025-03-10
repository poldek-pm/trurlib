
CFLAGS += -std=c23
CPPFLAGS = -I$(top_srcdir)/include/trurl -I$(top_srcdir)/include -I$(top_srcdir)
LDADD = $(top_builddir)/libtrurl.la

noinst_PROGRAMS = test_hash test_hash_func test_sort_func
perftests: $(noinst_PROGRAMS)

test_hash_func_LDADD = $(top_builddir)/libtrurl.la -lxxhash
