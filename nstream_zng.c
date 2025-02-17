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

#if HAVE_LIBZ_NG
#define WITH_GZFILEOP 1
#include <zlib-ng/zlib-ng.h>

#include "trurl_internal.h"
#include "nassert.h"
#include "nmalloc.h"
#include "nstream.h"
#include "ndie.h"

#define ZLIB_TRACE 0

static int do_ngz_read(void *stream, void *buf, size_t size)
{
    return zng_gzread(stream, buf, size);
}

static int do_ngz_write(void *stream, const void *buf, size_t size)
{
    return zng_gzwrite(stream, buf, size);
}

static char *do_ngz_gets(void *stream, char *buf, size_t size)
{
    return zng_gzgets(stream, buf, size);
}

static int do_ngz_flush(void *stream)
{
    return zng_gzflush(stream, Z_FULL_FLUSH);
}

static long do_ngz_tell(void *stream)
{
    return zng_gztell(stream);
}

static int do_ngz_getc(void *stream)
{
    register int c = zng_gzgetc((gzFile)stream);
    if (c == -1)
        return EOF;

    return c;
}

static int do_ngz_ungetc(int c, void *stream)
{
    register int cc = zng_gzungetc(c, stream);
    if (cc == -1)
        return EOF;
    n_assert(cc == c);
    return cc;
}

static int ngz_fseek_wrap(void *stream, long offset, int whence)
{
    z_off_t rc, off = offset;

#if ZLIB_TRACE_SEEK
    int seek = 0;
    switch (whence) {
    case SEEK_CUR:
        seek = zng_gztell(stream) + offset;
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

    if (seek < zng_gztell(stream)) {
        DBGF_F("%p backward from %lld => %ld (whence %d)\n", stream, zng_gztell(stream), offset, whence);
    }
#endif

    rc = zng_gzseek(stream, off, whence);

#if ZLIB_TRACE
    if (rc < 0) {
        int ec;
        printf("zlib_fseek: %s\n", zng_gzerror(stream, &ec));
    }
#endif

    if (rc > 0)
        rc = 0;

    return rc;
}

void *do_ngz_open(const char *path, const char *mode)
{
    gzFile gzstream;
    errno = 0;

    if ((gzstream = zng_gzopen(path, mode)) == NULL) {
        if (errno == 0) {
            if (Z_MEM_ERROR)
                errno = ENOMEM;
            else
                errno = EIO;
        }
    }

    return gzstream;
}

void n_stream_zlib_ng_init(tn_stream *st) {
    st->st_open  = do_ngz_open;
    st->st_dopen = (void *(*)(int, const char *))zng_gzdopen;
    st->st_read  = do_ngz_read;
    st->st_write = do_ngz_write;
    st->st_gets  = do_ngz_gets;
    st->st_getc  = do_ngz_getc;
    st->st_ungetc = do_ngz_ungetc;
    st->st_seek  = ngz_fseek_wrap;
    st->st_tell  = do_ngz_tell;
    st->st_flush = do_ngz_flush;
    st->st_close = (int (*)(void*))zng_gzclose;
}

#endif  /* HAVE_LIBZ_NG */
