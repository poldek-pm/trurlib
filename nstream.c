/*
  Copyright (C) 2002 - 2007 Pawel A. Gajda <mis@pld-linux.org>

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU Library General Public License, version 2 as
  published by the Free Software Foundation (see file COPYING for details).

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <zlib.h>

#include "trurl_internal.h"
#include "nassert.h"
#include "nmalloc.h"
#include "nstream.h"
#include "ndie.h"
#include "niobuf.h"

#define ZLIB_TRACE 0

static int do_gz_flush(void *stream)
{
    return gzflush(stream, Z_FULL_FLUSH);
}

static long do_gz_tell(void *stream)
{
    return gztell(stream);
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

static int do_gz_getc(void *stream)
{
    register int c = gzgetc((gzFile)stream);
    if (c == -1)
        return EOF;

    return c;
}

#if HAVE_GZUNGETC               /* zlib >= 1.2.0.2 */
static int do_gz_ungetc(int c, void *stream)
{
    register int cc = gzungetc(c, stream);
    if (cc == -1)
        return EOF;
    n_assert(cc == c);
    return cc;
}
#endif

static int zlib_fseek_wrap(void *stream, long offset, int whence)
{
    z_off_t rc, off = offset;

#if ZLIB_TRACE
    int seek = 0;
    switch (whence) {
    case SEEK_CUR:
        seek = gztell(stream) + offset;
        break;

    case SEEK_SET:
        seek = offset;
        break;

    case SEEK_END:
        n_die("SEEK_END is not supported\n");
        break;

    default:
        n_die("iobuf: unknown whence (%d)\n", whence);
        break;
    }

    if (seek < gztell(stream)) {
        DBGF_F("backward from %lld => %ld (whence %d)\n", gztell(stream), offset, whence);
    }
#endif

    rc = gzseek(stream, off, whence);

#if ZLIB_TRACE
    if (rc < 0) {
        int ec;
        printf("zlib_fseek: %s\n", gzerror(stream, &ec));
    }
#endif

    if (rc > 0)
        rc = 0;

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
            st->st_open  = (void *(*)(const char *, const char *))fopen;
            st->st_dopen = (void *(*)(int, const char *))fdopen;
            st->st_read  = (int (*)(void*, void*, size_t))do_s_read;
            st->st_write = (int (*)(void*, const void*, size_t))do_s_write;
            st->st_gets  = do_s_gets;
            st->st_getc  = (int (*)(void*))getc;
            st->st_ungetc = (int (*)(int, void*))ungetc;
            st->st_seek  = (int (*)(void*, long, int))fseek;
            st->st_tell  = (long (*)(void*))ftell;
            st->st_flush = (int (*)(void*))fflush;
            st->st_close = (int (*)(void*))fclose;
            break;

        case TN_STREAM_GZIO:
            st->st_open  = (void *(*)(const char *, const char *))gzopen;
            st->st_dopen = (void *(*)(int, const char *))gzdopen;
            st->st_read  = (int (*)(void*, void*, size_t))gzread;
            st->st_write = (int (*)(void*, const void*, size_t))gzwrite;
            st->st_gets  = (char *(*)(void*, char*, size_t))gzgets;
            st->st_getc  = do_gz_getc;
#if HAVE_GZUNGETC
            st->st_ungetc = do_gz_ungetc;
#else
            st->st_ungetc = NULL;
#endif
            st->st_seek  = (int (*)(void*, long, int))zlib_fseek_wrap;
            st->st_tell  = do_gz_tell;
            st->st_flush = do_gz_flush;
            st->st_close = (int (*)(void*))gzclose;
            break;

         case TN_STREAM_ZSTDIO:
            st->st_open  = (void *(*)(const char *, const char *))n_iobuf_open;
            st->st_dopen = (void *(*)(int, const char *))n_iobuf_dopen;
            st->st_read  = (int (*)(void*, void*, size_t))n_iobuf_read;
            st->st_write = (int (*)(void*, const void*, size_t))n_iobuf_write;
            st->st_gets  = (char *(*)(void*, char*, size_t))n_iobuf_gets;
            st->st_getc  = (int (*)(void*))getc;
            st->st_ungetc = NULL;
            st->st_seek  = (int (*)(void*, long, int))n_iobuf_seek;
            st->st_tell  = (long (*)(void*))n_iobuf_tell;
            st->st_flush = (int (*)(void*))n_iobuf_flush;
            st->st_close = (int (*)(void*))n_iobuf_close;
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

    if ((p = strrchr(path, '.'))) {
        if (strcmp(p, ".gz") == 0) {
            *type = TN_STREAM_GZIO;
        } else if (strcmp(p, ".zst") == 0) {
            *type = TN_STREAM_ZSTDIO;
        }
        real_type = *type;
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

        case TN_STREAM_ZSTDIO:
            if ((st->stream = n_iobuf_open(path, mode)) == NULL)
                rc = 0;
            break;

        default:
            n_assert(0);
    }

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

    if ((stream = st->st_dopen(fd, mode))) {
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
    return st->st_write(st->stream, buf, n);
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

inline
int n_stream_gets(tn_stream *st, char *buf, size_t size)
{
    if (st->st_gets(st->stream, buf, size))
		return strlen(buf);

	return 0;
}

#ifndef	MAX_CANON
# define	MAX_CANON	256
#endif
#undef getc
int n_stream_getline(tn_stream *st, char **bufptr, size_t size)
{
    char *buf = *bufptr;
    size_t n;

    if (buf == NULL) {
        buf = n_malloc(MAX_CANON);
        size = MAX_CANON;
    }

    *buf = '\0';

    n = n_stream_gets(st, buf, size);
    if (n < size - 1) {
        *bufptr = buf;
        return n;
    }

    while (1) {
        register int c;
        if (n == size - 1) {
            size *= 2;
            buf = n_realloc(buf, size);
        }
        c = st->st_getc(st->stream);
        if (c == EOF)
            break;

        buf[n++] = c;
        if (c == '\n')
            break;
    }

    buf[n] = '\0';
    *bufptr = buf;
    return n;
}
