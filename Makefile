# Makefile for trurlib
# $Id$
#
# Parameters
# - without_dbhash=[01]  (default 0)
# - modules=[on|off]     (default off)
# - prefix=/install/prefix

PROJ_DIR     = $(shell pwd)
VERSION      = $(shell cat VERSION)
INSTALL_ROOT = /usr/local

ifdef DESTDIR
	INSTALL_ROOT = $(DESTDIR)
endif

DEF_USE_N_ASSERT  = -DUSE_N_ASSERT
DEF_USE_XMALLOCS  = -DUSE_XMALLOCS

DEFINES  = $(DEF_USE_XMALLOCS) $(DEF_USE_N_ASSERT)
CPPFLAGS = -Itrurl $(DEFINES)
override CFLAGS += -pedantic -g -Wall -W $(DEFINES) 
LDLAGS 	 = 
CC 	 = gcc 
SHELL 	 = /bin/sh
RANLIB   = ranlib
AR	 = ar
STRIP	 = strip

NSTR_OBJECTS = \
	nstr_tok.o    \
	nstr_tokl.o   \
	nstr_len.o    \
	nstr_concat.o 


MOD_CPPFLAGS = -DMODULES -DMODULE

modules ?= off
ifeq ($(modules),off)
   NARRAY_OBJECTS  = narray.o 
   NLIST_OBJECTS   = nlist.o 
   NHASH_OBJECTS   = nhash.o

else 
   NARRAY_OBJECTS = \
	n_array_new.o \
	n_array_ctl_growth.o \
	n_array_clean.o \
	n_array_free.o \
	n_array_grow_priv.o \
	n_array_nth.o \
	n_array_remove_nth.o \
	n_array_set_nth.o \
	n_array_push.o \
	n_array_pop.o \
	n_array_shift.o \
	n_array_unshift.o \
	n_array_uniq_ex.o \
	n_array_eq_ex.o \
	n_array_dup.o \
	n_array_sorts.o \
	n_array_bsearch_ex.o \
	n_array_map.o \
	n_array_map_arg.o \
	n_array_dump_stats.o

   NLIST_OBJECTS = \
	n_list_new.o \
	n_list_free.o \
	n_list_push.o \
	n_list_pop.o \
	n_list_shift.o \
	n_list_unshift.o \
	n_list_remove_nth.o \
	n_list_remove_ex.o \
	n_list_lookup_ex.o \
	n_list_contains_ex.o \
	n_list_nth.o \
	n_list_iterator.o \
	n_list_size.o \
	n_list_map_arg.o

   NHASH_OBJECTS = \
	n_hash_new.o \
	n_hash_ctl.o \
	n_hash_clean.o \
	n_hash_put.o \
	n_hash_get.o \
	n_hash_exists.o \
	n_hash_remove.o \
	n_hash_free.o \
	n_hash_map.o \
	n_hash_map_arg.o
endif


OBJECTS = \
	xmalloc.o        \
	nassert.o        \
	nbuf.o		 \
	$(NARRAY_OBJECTS)\
        $(NLIST_OBJECTS) \
	$(NHASH_OBJECTS) \
	trurl_die.o      \
	trurl_cmpf.o     \
	$(NSTR_OBJECTS)

SHOBJECTS = $(addsuffix s, $(OBJECTS))

# fix-info-dir subset stuff
FIXIDIRSRCS        =  xmalloc.c nassert.c trurl_die.c trurl_cmpf.c
FIXIDIROBJS        =  $(NARRAY_OBJECTS) $(FIXIDIRSRCS:.c=.o)
FIXIDIRARCH        =  trurlib-$(VERSION)fid
FIXIDIRARCH_TARGET =  fid-arch
FIXIDIRTARGET      =  narray.a

HEADERS = \
	nassert.h   \
	nhash.h     \
	nstr.h      \
	tfn_types.h \
	narray.h    \
	nlist.h     \
	nbuf.h	    \
	trurl.h     \
	xmalloc.h


LIBNAME     =  trurl
STATIC_LIB  =  lib$(LIBNAME).a
SHARED_LIB  =  lib$(LIBNAME).so.$(VERSION)
SONAME      =  lib$(LIBNAME).so.$(shell cut -f 1 -d . VERSION)
TEST_PROGS = \
		test_common \
		test_array  \
	        test_list   \
		test_hash   \
		test_nstr  


without_dbhash ?= 0
ifeq ($(without_dbhash),0)
  OBJECTS    += ndbhash.o
  HEADERS    += ndbhash.h
  TEST_PROGS += test_dbhash
  LDFLAGS    += -ldb1  
endif


####### Implicit rules
n_array_%.o: narray.c
	$(CC) -c $(CFLAGS) $(CPPFLAGS) $(MOD_CPPFLAGS)_n_array_$* -o $@ $<

n_array_%.os: narray.c
	$(CC) -c -fPIC $(CFLAGS) $(CPPFLAGS) $(MOD_CPPFLAGS)_n_array_$* -o $@ $<


n_list_%.o: nlist.c
	$(CC) -c $(CFLAGS) $(CPPFLAGS) $(MOD_CPPFLAGS)_n_list_$* -o $@ $<


n_list_%.os: nlist.c
	$(CC) -c -fPIC $(CFLAGS) $(CPPFLAGS) $(MOD_CPPFLAGS)_n_list_$* -o $@ $<


n_hash_%.o: nhash.c
	$(CC) -c $(CFLAGS) $(CPPFLAGS) $(MOD_CPPFLAGS)_n_hash_$* -o $@ $<


n_hash_%.os: nhash.c
	$(CC) -c -fPIC $(CFLAGS) $(CPPFLAGS) $(MOD_CPPFLAGS)_n_hash_$* -o $@ $<

%.o:	%.c
	$(CC) -c $(CFLAGS) $(CPPFLAGS) -o $@ $<

%.os:	%.c
	$(CC) -c -fPIC $(CFLAGS) $(CPPFLAGS) -o $@ $<


test_%:  test_%.o 
	$(CC) $(CFLAGS) $< -o $@ $(LFLAGS) -L. -ltrurl $(LDFLAGS) 

all: static

static: symlink $(STATIC_LIB) 
shared: symlink $(SHARED_LIB)
tests: $(TEST_PROGS)


$(STATIC_LIB): $(OBJECTS)
	$(AR) cr $@ $?
	$(RANLIB) $@

$(SHARED_LIB): $(SHOBJECTS)
	gcc -shared -Wl,-soname=$(SONAME) $(CFLAGS) -o $@ $?

# for fix-info-dir
$(FIXIDIRTARGET): $(FIXIDIROBJS)
	$(AR) cr $@ $?
	$(RANLIB) $@

symlink: 
	@if [ ! -d trurl ]; then ln . trurl -s; fi;

dep:
	gcc -MM $(CPPFLAGS) *.c > .depend
	@for f in $(TEST_PROGS); do                    \
		echo "$$f: $(STATIC_LIB)" >> .depend;  \
	done;                                          

tags: 
	etags -e *.c *.h

TAGS:   tags

install: 
	install -d 755 $(INSTALL_ROOT)/lib/
	install -m 644 $(STATIC_LIB) $(INSTALL_ROOT)/lib/
	install -s -m 755 $(SHARED_LIB) $(INSTALL_ROOT)/lib/
	ln -sf $(SHARED_LIB) $(INSTALL_ROOT)/lib/libtrurl.so
	install -d 755 $(INSTALL_ROOT)/include/trurl
	install -m 644 $(HEADERS) $(INSTALL_ROOT)/include/trurl

.PHONY: clean distclean backup arch dist

clean: TEST_PROGS += test_dbhash
clean:  
	-rm -f core *.o *.os *.bak *~ *% *\# 
	-rm -f $(STATIC_LIB) $(TEST_PROGS) $(SHARED_LIB) trurl
	-rm -f $(FIXIDIRTARGET) 
	-rm -rf $(FIXIDIRARCH)

distclean: clean
	-rm -f .depend TAGS gmon.out .swp 

$(FIXIDIRARCH_TARGET): clean narray.o
	 @DEPS=`gcc -MM $(CPPFLAGS) $(FIXIDIRSRCS) narray.c | \
	 sed -e 's|^.*: ||g' -e 's|\\\||g' ` ; \
	 echo $$DEPS; \
	 mkdir $(FIXIDIRARCH); \
	 cp -a $$DEPS $(FIXIDIRSRCS) Makefile VERSION $(FIXIDIRARCH); \
	 tar cvpzf /tmp/$(FIXIDIRARCH).tar.gz $(FIXIDIRARCH);               \
	 rm -rf $(FIXIDIRARCH); 

backup:
	@cd $(PROJ_DIR); \
	ARCHDIR=`basename  $(PROJ_DIR)`-ARCH; \
	ARCHNAME=`basename  $(PROJ_DIR)`-`date +%Y.%m.%d-%H.%M`; \
	cd ..; \
	mkdir $$ARCHDIR || true; \
	cd $$ARCHDIR && mkdir $$ARCHNAME || exit 1; \
	cd .. ;\
	cp -a $(PROJ_DIR) $$ARCHDIR/$$ARCHNAME ;\
	cd $$ARCHDIR ;\
	tar cvpzf $$ARCHNAME.tgz $$ARCHNAME && rm -rf $$ARCHNAME;   \
	md5sum $$ARCHNAME.tgz > $$ARCHNAME.md5;                     \
	if [ $(cparch)x = "1x" ]; then                              \
	        mkdir $(backupdir)/copy || true;                    \
		cp -v $$HOME/$$ARCHDIR/$$ARCHNAME.tgz $(backupdir); \
		cp -v $$HOME/$$ARCHDIR/$$ARCHNAME.tgz $(backupdir)/copy; \
		cd $(backupdir) || exit 1;                         \
		md5sum --check $$HOME/$$ARCHDIR/$$ARCHNAME.md5;    \
		cd copy || exit 1;                                 \
		md5sum --check $$HOME/$$ARCHDIR/$$ARCHNAME.md5;    \
	fi

arch : distclean  backup 

misarch: distclean 
	$(MAKE) -C . backup cparch=1 backupdir=/z/

#
# Make dist archives of $(PROJ_DIR). Archives are stored in TMPDIR (see below)
# as $(PROJ_DIR)-`cat VERSION`.tar.[gz, bz2]
# 
dist:   distclean
	@cd $(PROJ_DIR)                       ;\
	TMPDIR=/tmp                           ;\
	REV=`cat VERSION`                     ;\
	CURDIR=`basename  $(PROJ_DIR)`        ;\
	DISTDIR=$$TMPDIR/$$CURDIR-$$REV/      ;\
	rm -rf $$DISTDIR                      ;\
	mkdir $$DISTDIR                       ;\
	cp -a * $$DISTDIR                     ;\
	for f in `cat .cvsignore`; do          \
             rm -rf $$DISTDIR/$$f             ;\
        done                                  ;\
	cd $$TMPDIR                           ;\
	arch_name=`basename $$DISTDIR`        ;\
	tar cvpf $$arch_name.tar $$arch_name  ;\
	gzip -9f $$arch_name.tar && rm -rf $$DISTDIR

rpm:    dist
	@rpm -ta /tmp/trurlib-$(VERSION).tar.gz 


ifeq (.depend,$(wildcard .depend))
include .depend
endif
