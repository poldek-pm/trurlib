# Makefile for trurlib
# $Id$
#
#
PROJ_DIR=$(shell pwd)

DEF_USE_N_ASSERT  = -DUSE_N_ASSERT
DEF_USE_XMALLOCS  = -DUSE_XMALLOCS
LIBC             = -DGLIBC_2_1

DEFINES  = $(DEF_USE_XMALLOCS) $(DEF_USE_N_ASSERT) $(LIBC)
INCLUDE  = -Itrurl
CFLAGS	 = -O2 -pedantic -g -Wall -W $(DEFINES) 
LFLAGS 	 = 
LIBS	 = -ldb #-lccmalloc -ldl
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


OBJECTS = \
	xmalloc.o    \
	nassert.o    \
	nhash.o      \
	ndbhash.o    \
	nlist.o      \
	narray.o     \
	trurl_die.o  \
	$(NSTR_OBJECTS)

TARGET	   =  libtrurl.a

TEST_PROGS = \
		test_common \
		test_array  \
	        test_list   \
		test_hash   \
	        test_dbhash \
		test_nstr  


####### Implicit rules

%.o:	%.cc
	$(CC) -c $(CFLAGS) $(INCLUDE) -o $@ $<

%.o:	%.c
	$(CC) -c $(CFLAGS) $(INCLUDE) -o $@ $<

test_%:  test_%.o 
	$(CC) $(CFLAGS) $< -o $@ $(LFLAGS) -L. -ltrurl $(LIBS) 


all: symlink $(TARGET) $(TEST_PROGS)


$(TARGET): $(OBJECTS)
	$(AR) cr $(TARGET) $(OBJECTS)
	$(RANLIB) $(TARGET)

symlink: 
	@if [ ! -d trurl ]; then ln . trurl -s; fi;

dep:
	gcc -MM $(DEFINES) $(INCLUDE) *.c >.depend

tags: 
	etags -e *.c *.h

TAGS:   tags


.PHONY: clean distclean backup arch dist

clean:
	-rm -f core *.o *.bak *~ *% *\#
	-rm -f $(TARGET) $(TEST_PROGS) 


distclean: clean
	-rm -f .depend TAGS gmon.out .swp 

#
# Make copy of $(PROJ_DIR). Copies are stored in ../$(PROJ_DIR)-ARCH as  
# $(PROJ_DIR)-DD.MM.YY-HH.MM.tgz 
backup: 
	@cd $(PROJ_DIR)                       ;\
	CURDIR=`basename  $(PROJ_DIR)`        ;\
	CURDATE=`date +%d.%m.%y-%H.%M`        ;\
	ARCHDIR=$$CURDIR-ARCH                 ;\
	ARCHNAME=$$CURDIR-$$CURDATE           ;\
	cd $(PROJ_DIR)/..                     ;\
	mkdir $$ARCHDIR  2>/dev/null || true  ;\
	mkdir $$ARCHDIR/$$ARCHNAME            ;\
	cp -ra $$CURDIR $$ARCHDIR/$$ARCHNAME  ;\
	cd $$ARCHDIR                          ;\
        tar cvpzf $$ARCHNAME.tgz $$ARCHNAME  



arch :	distclean  backup 

#
# Make dist archives of $(PROJ_DIR). Archives are stored in TMPDIR (see below)   
# as $(PROJ_DIR)-`cat VERSION`.tar.[gz, bz2]
# 
dist:
	@cd $(PROJ_DIR)                       ;\
	TMPDIR=/tmp                           ;\
	REV=`cat VERSION`                     ;\
	CURDIR=`basename  $(PROJ_DIR)`        ;\
	DISTDIR=$$TMPDIR/$$CURDIR-$$REV/      ;\
	rm -rf $$DISTDIR                      ;\
	mkdir $$DISTDIR                       ;\
	cp -a * $$DISTDIR                     ;\
	cd $$TMPDIR                           ;\
	arch_name=`basename $$DISTDIR`        ;\
	tar cvpf $$arch_name.tar $$arch_name  ;\
	bzip2 -9 -k $$arch_name.tar           ;\
	gzip -9 $$arch_name.tar


ifeq (.depend,$(wildcard .depend))
include .depend
endif
