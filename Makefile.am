# $Id$

AUTOMAKE_OPTIONS = foreign

# use libtool versioning 
TRURL_VERINFO   = 4:0:4
CPPFLAGS  = -I$(top_srcdir)/include/trurl -I$(top_srcdir)/include
INCLUDES  = 
AM_CFLAGS = @AM_CFLAGS@

lib_LTLIBRARIES  = libtrurl.la
libtrurl_la_LDFLAGS = -version-info $(TRURL_VERINFO)

NARRAY_SRCS  = \
	n_array_bsearch_ex.c    \
        n_array_clean.c \
	n_array_clone.c \
        n_array_ctl.c   \
        n_array_dump_stats.c    \
        n_array_eq_ex.c \
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
        n_hash_free.c   \
        n_hash_get.c    \
        n_hash_int.h    \
        n_hash_map.c    \
        n_hash_map_arg.c       \
        n_hash_new.c    \
        n_hash_put.c    \
        n_hash_remove.c \
	n_hash_stats.c  \
	n_hash_keys.c   \
	n_hash_dup.c

NSTR_SRCS = \
	nstr_tok.c     \
	nstr_tokl.c    \
	nstr_len.c     \
	nstr_concat.c  \
	nstr_cpy.c     \
	nstr_basenam.c


libtrurl_la_SOURCES = \
	trurl_cmpf.c     \
	trurl_internal.h \
	ndie.c		 \
	nmalloc.c        \
	xmalloc.c        \
	nassert.c        \
	nbuf.c		 \
	nstream.c        \
	nstore.c         \
	$(NARRAY_SRCS)   \
        $(NLIST_SRCS)    \
	$(NHASH_SRCS)    \
	$(NSTR_SRCS)     \
	ntuple.c

pkgincludedir = $(includedir)/trurl
pkginclude_HEADERS = \
		include/trurl/nassert.h    \
		include/trurl/nhash.h      \
		include/trurl/nstr.h       \
		include/trurl/tfn_types.h  \
		include/trurl/narray.h     \
		include/trurl/nlist.h      \
		include/trurl/nbuf.h	   \
		include/trurl/trurl.h      \
		include/trurl/ndie.h	   \
		include/trurl/nmalloc.h    \
		include/trurl/n_snprintf.h \
		include/trurl/nstream.h    \
		include/trurl/nstore.h     \
		include/trurl/n2h.h        \
		include/trurl/n_obj_ref.h  \
		include/trurl/ntuple.h

EXTRA_DIST = Makefile.extra trurlib.spec trurlib.spec.in \
			 test_common.c \
			 test_array.c \
			 test_list.c \
			 test_hash.c \
			 test_nstr.c \
			 test_stream.c \
			 test_obstack.c \
			 README NEWS

EXTRA_PROGRAMS = \
		test_common \
		test_array  \
	        test_list   \
		test_hash   \
		test_nstr   \
		test_stream \
		test_obstack

test_common_LDADD = libtrurl.la
test_array_LDADD = libtrurl.la
test_list_LDADD = libtrurl.la
test_hash_LDADD = libtrurl.la
test_nstr_LDADD = libtrurl.la
test_stream_LDADD =  libtrurl.la -lz
test_obstack_LDADD =  libtrurl.la
MAINTAINERCLEANFILES =	mkinstalldirs install-sh missing *.tar.gz *.spec \
			config.h.in configure Makefile.in config.h \
			config.sub config.guess aclocal.m4 \
			libtool ltconfig ltmain.sh stamp-h* depcomp

DISTCLEANFILES = TAGS gmon.out trurl $(distdir)/.deps libtrurl.a
CLEANFILES = $(EXTRA_PROGRAMS) core *.o *.bak *~ *% *\# \#* 

symlink: 
	@if [ ! -d trurl ]; then ln . trurl -s; fi;

tests:  $(EXTRA_PROGRAMS)

all-local: 
	@ln -sf .libs/libtrurl.a libtrurl.a

#dist-hook:
#	rm -rf $(distdir)/.deps

#clean-local:
#	-rm -f core *.o *.bak *~ *% *\# TAGS gmon.out trurl \#*
#	-rm -f $(EXTRA_PROGRAMS)

include Makefile.extra
