# Process this file with autoconf to produce a configure script.

m4_define([trurlib_major_version], [0])
m4_define([trurlib_minor_version], [50])
m4_define([trurlib_micro_version], [0])
m4_define([trurlib_version],
	  [trurlib_major_version.trurlib_minor_version.trurlib_micro_version])

AC_PREREQ([2.63])
AC_INIT([trurlib], [trurlib_version], [http://bugs.launchpad.net/poldek])
AC_CONFIG_SRCDIR([n_hash_keys.c])
AC_CONFIG_HEADERS([config.h])

AM_INIT_AUTOMAKE([1.11 foreign no-dist-gzip dist-xz])
AM_MAINTAINER_MODE([enable])
AM_SILENT_RULES([no])

# Checks for programs.
m4_ifdef([LT_INIT], [LT_INIT], [AC_PROG_LIBTOOL])
AC_PROG_CC
AC_PROG_CPP
AC_PROG_INSTALL
AC_PROG_LN_S
AC_PROG_MAKE_SET
AC_PROG_RANLIB

# libtool versioning
LT_CURRENT=7
LT_REVISION=0
LT_AGE=7
AC_SUBST(LT_CURRENT)
AC_SUBST(LT_REVISION)
AC_SUBST(LT_AGE)

AM_CFLAGS="-Wall -Wextra -Wformat-security"
AC_SUBST(AM_CFLAGS)

# Checks for header files.
AC_HEADER_STDC
AC_CHECK_HEADERS([limits.h netinet/in.h stddef.h stdint.h stdlib.h string.h strings.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_C_INLINE
AC_TYPE_INT16_T
AC_TYPE_SIZE_T
AC_TYPE_UINT16_T
AC_TYPE_UINT32_T
AC_TYPE_UINT8_T

# Checks for library functions.
AC_FUNC_ALLOCA
AC_FUNC_MALLOC
AC_FUNC_REALLOC
AC_CHECK_FUNCS([memmove memset strchr strrchr strspn])

dnl unaligned accesses (XXX: todo cross compliation), stolen from libpcap
case "$host_cpu" in
    alpha*|arm*|hp*|mips*|sh*|sparc*|ia64|nv1)
        AC_DEFINE(HAVE_CPU_UNALIGNED_ACCESS, 0, 
        	  [Define to 0 if CPU does not like unaligned access.])
        ;;
    *) 
        AC_DEFINE(HAVE_CPU_UNALIGNED_ACCESS, 1)
        ;;
esac

dnl Disabled cookies usage; they make too much troubles.
dnl AC_CHECK_FUNCS(fopencookie,,,$CONF_IN_LDFLAGS)

dnl stolen from autoconf
AN_FUNCTION([obstack_init], [AC_FUNC_OBSTACK])
AN_IDENTIFIER([obstack],    [AC_FUNC_OBSTACK])

AC_DEFUN([AC_FUNC_OBSTACK],
	 [AC_LIBSOURCES([obstack.h, obstack.c])
	  AC_CACHE_CHECK([for obstacks], func_cv_obstack,
			 [AC_LINK_IFELSE(
			  [AC_LANG_PROGRAM([[@%:@include "obstack.h"]],
                          [[struct obstack *mem; obstack_free(mem,(char *) 0)]])],
            		  [func_cv_obstack=yes],
            		  [func_cv_obstack=no])
            		 ])
	  if test $func_cv_obstack = yes; then
	    AC_DEFINE(HAVE_OBSTACK, 1, [Define to 1 if libc includes obstacks.])
	    AM_CONDITIONAL(ENABLE_SELF_OBSTACK, false)
	  else
	    AM_CONDITIONAL(ENABLE_SELF_OBSTACK, true)
	  fi
])

AC_FUNC_OBSTACK

AC_CHECK_LIB(z, gzopen,,[AC_MSG_ERROR(["missing zlib"])])
AC_CHECK_LIB(z, gzungetc,,[AC_MSG_WARN(["upgrade zlib (>= 1.2.0.3 is recommended)"])])

AC_CONFIG_FILES([
Makefile
data/Makefile
data/trurlib.pc
lib/Makefile
])
AC_OUTPUT
