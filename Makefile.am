# $Id$

AUTOMAKE_OPTIONS = foreign

# use libtool versioning 
TRURL_VERINFO   = 2:0:2
CFLAGS    = @CFLAGS@
INCLUDES  = 

lib_LTLIBRARIES  = libtrurl.la
libtrurl_la_LDFLAGS = -version-info $(TRURL_VERINFO)

NARRAY_SRCS  = \
	n_array_bsearch_ex.c    \
        n_array_clean.c \
	n_array_clone.c \
        n_array_ctl.c   \
        n_array_dump_stats.c    \
        n_array_dup.c   \
        n_array_eq_ex.c \
        n_array_free.c  \
        n_array_grow_priv.c     \
        n_array_int.h   \
        n_array_map.c   \
        n_array_map_arg.c       \
        n_array_new.c   \
        n_array_nth.c   \
        n_array_pop.c   \
        n_array_push.c  \
	n_array_remove_ex.c     \
        n_array_remove_nth.c    \
        n_array_set_nth.c       \
        n_array_shift.c \
        n_array_sorts.c \
        n_array_uniq_ex.c       \
        n_array_unshift.c

NLIST_SRCS = \
	n_list_int.h \
	n_list_new.c \
	n_list_free.c \
	n_list_push.c \
	n_list_pop.c \
	n_list_shift.c \
	n_list_unshift.c \
	n_list_remove_nth.c \
	n_list_remove_ex.c \
	n_list_lookup_ex.c \
	n_list_contains_ex.c \
	n_list_nth.c \
	n_list_iterator.c \
	n_list_size.c \
	n_list_map_arg.c

NHASH_SRCS = \
	n_hash_clean.c  \
        n_hash_ctl.c    \
        n_hash_exists.c \
        n_hash_free.c   \
        n_hash_get.c    \
        n_hash_int.h    \
        n_hash_map.c    \
        n_hash_map_arg.c       \
        n_hash_new.c    \
        n_hash_put.c    \
        n_hash_remove.c \
        n_hash_stats.c  \
	hash-string.h 

NSTR_SRCS = \
	nstr_tok.c     \
	nstr_tokl.c    \
	nstr_len.c     \
	nstr_concat.c  \
	nstr_cpy.c     \
	nstr_basenam.c


libtrurl_la_SOURCES = \
	trurl_die.c      \
	trurl_cmpf.c     \
	trurl_internal.h \
	nmalloc.c        \
	nassert.c        \
	nbuf.c		 \
	$(NARRAY_SRCS)   \
        $(NLIST_SRCS)    \
	$(NHASH_SRCS)    \
	$(NSTR_SRCS)

pkgincludedir = $(includedir)/trurl
pkginclude_HEADERS = \
		nassert.h   \
		nhash.h     \
		nstr.h      \
		tfn_types.h \
		narray.h    \
		nlist.h     \
		nbuf.h	    \
		trurl.h     \
		nmalloc.h   \
		n_snprintf.h

EXTRA_DIST = Makefile.extra trurlib.spec trurlib.spec.in test_*.c README NEWS

EXTRA_PROGRAMS = \
		test_common \
		test_array  \
	        test_list   \
		test_hash   \
		test_nstr  

test_common_LDADD = libtrurl.la
test_array_LDADD = libtrurl.la
test_list_LDADD = libtrurl.la
test_hash_LDADD = libtrurl.la
test_nstr_LDADD = libtrurl.la

MAINTAINERCLEANFILES =	mkinstalldirs install-sh missing *.tar.gz *.spec \
			config.h.in configure Makefile.in config.h \
			config.sub config.guess aclocal.m4 \
			libtool ltconfig ltmain.sh stamp-h* 

DISTCLEANFILES = TAGS gmon.out trurl $(distdir)/.deps libtrurl.a
CLEANFILES = $(EXTRA_PROGRAMS) core *.o *.bak *~ *% *\# \#* 

symlink: 
	@if [ ! -d trurl ]; then ln . trurl -s; fi;

tests:  symlink $(EXTRA_PROGRAMS)

all-local: symlink
	@ln -sf .libs/libtrurl.a libtrurl.a

#dist-hook:
#	rm -rf $(distdir)/.deps

#clean-local:
#	-rm -f core *.o *.bak *~ *% *\# TAGS gmon.out trurl \#*
#	-rm -f $(EXTRA_PROGRAMS)

include Makefile.extra
