/*
  Copyright (C) 2002 Pawel A. Gajda <mis@k2.net.pl>

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU Library General Public License, version 2 as
  published by the Free Software Foundation (see file COPYING for details).

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/

/*
  $Id$
*/

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#ifdef HAVE_FOPENCOOKIE
# define _GNU_SOURCE 1
#endif

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <zlib.h>

#include "nassert.h"
#include "nmalloc.h"
#include "nstream.h"
#include "ndie.h"

#define ZLIB_TRACE 0

#ifdef HAVE_FOPENCOOKIE
#ifndef __GLIBC_MINOR__
# error "glibc2 or later is required"
#endif

#ifndef __GLIBC_PREREQ
# if defined __GLIBC__ && defined __GLIBC_MINOR__
#  define __GLIBC_PREREQ(maj, min) \
       ((__GLIBC__ << 16) + __GLIBC_MINOR__ >= ((maj) << 16) + (min))
# else
#  define __GLIBC_PREREQ(maj, min) 0
# endif
#endif /* __GLIBC_PREREQ */

#if __GLIBC_PREREQ(2,2)
static
int gzfseek(void *stream, _IO_off64_t *offset, int whence)
{
    z_off_t rc, off = *offset;
    
    rc = gzseek(stream, off, whence);
    if (rc >= 0)
        rc = 0;
#if ZLIB_TRACE
    printf("zfseek (%p, %ld, %lld, %d) = %d\n", stream, off, *offset, whence, rc);
#endif    
    return rc;
}

#else  /* glibc < 2.2 */
static
int gzfseek(void *stream, _IO_off_t offset, int whence) 
{
    z_off_t rc;
    
    rc = gzseek(stream, offset, whence);
#if ! __GLIBC_PREREQ(2,1)       /* AFAIK glibc2.1.x cookie streams required
                                   offset to be returned */
    if (rc >= 0) 
        rc = 0;
#endif
    return rc;
}
#endif /* __GLIBC_PREREQ(2,2) */

#if ZLIB_TRACE
static
int gzread_wrap(void *stream, char *buf, size_t size)
{
    int rc;
    rc = gzread(stream, buf, size);
    printf("zread (%p, %d) = %d (%m)\n", stream, size, rc);
    return rc;
}
#endif

static cookie_io_functions_t gzio_cookie = {
#if ZLIB_TRACE    
    (cookie_read_function_t*)gzread_wrap,
#else
    (cookie_read_function_t*)gzread,
#endif    
    (cookie_write_function_t*)gzwrite,
    gzfseek,
    (cookie_close_function_t*)gzclose
};

#endif /* HAVE_FOPENCOOKIE */


static int do_gz_flush(void *stream)
{
    return gzflush(stream, Z_FULL_FLUSH);
}


static int do_s_read(void *stream, void *buf, size_t size)
{
    return fread(buf, 1, size, stream);
}

static int do_s_write(void *stream, const void *buf, size_t size)
{
    return fwrite(buf, 1, size, stream);
}

static char *do_s_gets(void *stream, char *buf, size_t size)
{
    return fgets(buf, size, stream);
}


static int zlib_fseek_wrap(void *stream, long offset, int whence) 
{
    z_off_t rc, off = offset;

    rc = gzseek(stream, off, whence);
    if (rc > 0)
        rc = 0;

#if ZLIB_TRACE
    if (rc < 0) {
        int ec;
        printf("zlib_fseek: %s\n", gzerror(stream, &ec));
    }
#endif 
    return rc;
}



static tn_stream *n_stream_new(int type)
{
    tn_stream *st;
    
    st = n_malloc(sizeof(*st));
    st->stream = NULL;
    st->fd = -1;
    st->type = type;
    st->_write_hook = NULL;
    st->_write_hook_arg = NULL;
    
    switch (type) {
        case TN_STREAM_STDIO:
            st->open  = (void *(*)(const char *, const char *))fopen;
            st->dopen = (void *(*)(int, const char *))fdopen;
            st->read  = (int (*)(void*, void*, size_t))do_s_read;
            st->write = (int (*)(void*, const void*, size_t))do_s_write;
            st->gets  = do_s_gets;
            st->seek  = (int (*)(void*, long, int))fseek;
            st->tell  = (long (*)(void*))ftell;
            st->flush = (int (*)(void*))fflush;
            st->close = (int (*)(void*))fclose;
            break;
            
        case TN_STREAM_GZIO:
            st->open  = gzopen;
            st->dopen = gzdopen;
            st->read  = gzread;
            st->write = (int (*)(void*, const void*, size_t))gzwrite;
            st->gets  = (char *(*)(void*, char*, size_t))gzgets;
            st->seek  = (int (*)(void*, long, int))zlib_fseek_wrap;
            st->tell  = (long (*)(void*))gztell;
            st->flush = do_gz_flush;
            st->close = gzclose;
            break;
            
        default:
            n_die("%d: unknown stream type\n", type);
            n_assert(0);
            break;
    }

    return st;
}

static int determine_type(const char *path, int *type) 
{
    const char *p;
    int real_type;


    if (*type == TN_STREAM_UNKNOWN)
        *type = TN_STREAM_STDIO;

    real_type = *type;
    
#ifdef HAVE_FOPENCOOKIE
    if (*type == TN_STREAM_GZIO) {
        *type = TN_STREAM_STDIO;
        real_type = TN_STREAM_GZIO;
    }
    
#endif
    
    if ((p = strrchr(path, '.')) && strcmp(p, ".gz") == 0) {
#ifdef HAVE_FOPENCOOKIE
        *type = TN_STREAM_STDIO;
        real_type = TN_STREAM_GZIO;
#else
        *type = TN_STREAM_GZIO;
        real_type = TN_STREAM_GZIO;
#endif        
    }

    return real_type;
}



static gzFile do_gz_open(const char *path, const char *mode)
{
    gzFile gzstream;
    errno = 0;
    
    if ((gzstream = gzopen(path, mode)) == NULL) {
        if (errno == 0) {
            if (Z_MEM_ERROR)
                errno = ENOMEM;
            else
                errno = EIO;
        }
    }

    return gzstream;
}


/* RET: bool */
static int do_open(tn_stream *st, const char *path, const char *mode,
                   int type, int real_type) 
{ 
    int rc = 1;

    
    real_type = determine_type(path, &type);
    
    switch (real_type) {
        case TN_STREAM_STDIO: 
            if ((st->stream = fopen(path, mode)) == NULL)
                rc = 0;
            break;

        case TN_STREAM_GZIO:
            if ((st->stream = do_gz_open(path, mode)) == NULL)
                rc = 0;
            break;

        default:
            n_assert(0);
    }

#ifdef HAVE_FOPENCOOKIE 
    if (real_type != type) {
        void *stream = NULL;
        switch (type) {
            case TN_STREAM_STDIO:
                stream = fopencookie(st->stream, mode, gzio_cookie);
                if (stream == 0) {
                    gzclose(st->stream);
                    rc = 0;
                    errno = EIO;
                    
                } else {
                    st->stream = stream;
                    fseek(st->stream, 0, SEEK_SET); /* glibc BUG (?) */
                    st->type = type;
                };
                break;

            default:
				printf("type = %d, %d\n", type, real_type);
                n_assert(0);
                break;
        }
    }
#endif
    
    return rc;
}


tn_stream *n_stream_open(const char *path, const char *mode, int type)
{
    tn_stream *st;
    int real_type;

    
    real_type = determine_type(path, &type);

    if ((st = n_stream_new(type)) == NULL)
        return NULL;

    if (!do_open(st, path, mode, type, real_type)) {
        n_stream_close(st);
        st = NULL;
    }
    
    return st;     
}

tn_stream *n_stream_dopen(int fd, const char *mode, int type)
{
    tn_stream *st;
    void *stream;
    
    if ((st = n_stream_new(type)) == NULL)
        return NULL;
    
    if ((stream = st->dopen(fd, mode))) {
        st->stream = stream;
        st->fd = fd;
        
    } else {
        n_stream_close(st);
        st = NULL;
    } 
    
    return st;     
}


int n_stream_vprintf(tn_stream *st, const char *fmt, va_list ap) 
{
    char buf[1024 * 32];
    int  n;

    n = vsnprintf(buf, sizeof(buf), fmt, ap);
    return st->write(st->stream, buf, n);
}


int n_stream_printf(tn_stream *st, const char *fmt, ...) 
{
    va_list ap;
    int n;

    
    va_start(ap, fmt);
    n = n_stream_vprintf(st, fmt, ap);
    va_end(ap);

    return n;
}


int n_stream_gets(tn_stream *st, char *buf, size_t size)
{
    if (st->gets(st->stream, buf, size))
		return strlen(buf);
	
	return 0;
}

