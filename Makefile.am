NULL =

AUTOMAKE_OPTIONS = foreign

SUBDIRS   = data lib tests
DIST_SUBDIRS = $(SUBDIRS)

# use libtool versioning
CPPFLAGS  = -I$(top_srcdir)/include/trurl -I$(top_srcdir)/include
AM_CPPFLAGS  =
AM_CFLAGS = @AM_CFLAGS@


lib_LTLIBRARIES =						\
	libtrurl.la						\
	$(NULL)

libtrurl_la_LDFLAGS = 						\
	-version-info $(LT_CURRENT):$(LT_REVISION):$(LT_AGE)	\
	-export-symbols $(top_srcdir)/libtrurl.sym		\
	$(NULL)

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
	nstr_basenam.c \
	nstr.c


libtrurl_la_SOURCES = \
	trurl_cmpf.c     \
	trurl_internal.h \
	ndie.c		 \
	nmalloc.c        \
	xmalloc.c        \
	nassert.c        \
	nbuf.c		 \
	nstream.c        \
	nstream_zng.c	 \
	nstream_z.c	 \
	nstore.c         \
	$(NARRAY_SRCS)   \
	$(NLIST_SRCS)    \
	$(NHASH_SRCS)    \
	$(NSTR_SRCS)     \
	ntuple.c         \
	niobuf.c	 \
	niobuf_zstd.c	 \
	niobuf_int.h


libtrurl_la_LIBADD = lib/libsys.la

pkgincludedir = $(includedir)/trurl
pkginclude_HEADERS = \
		include/trurl/nassert.h    \
		include/trurl/nhash.h      \
		include/trurl/nstr.h       \
		include/trurl/tfn_types.h  \
		include/trurl/narray.h     \
		include/trurl/nlist.h      \
		include/trurl/nbuf.h	   \
		include/trurl/niobuf.h	   \
		include/trurl/trurl.h      \
		include/trurl/ndie.h	   \
		include/trurl/nmalloc.h    \
		include/trurl/n_snprintf.h \
		include/trurl/nstream.h    \
		include/trurl/nstore.h     \
		include/trurl/n2h.h        \
		include/trurl/n_obj_ref.h  \
		include/trurl/ntuple.h     \
	        include/trurl/n_check.h

EXTRA_DIST = Makefile.extra trurlib.spec trurlib.spec.in libtrurl.sym \
			 README NEWS

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
	@ln -sf .libs/libtrurl.so libtrurl.so

include Makefile.extra
