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
#if HAVE_LIBZ
#include <zlib.h>

#include "trurl_internal.h"
#include "nassert.h"
#include "nmalloc.h"
#include "nstream.h"
#include "ndie.h"

#define ZLIB_TRACE 0

static int do_gz_read(void *stream, void *buf, size_t size)
{
    return gzread(stream, buf, size);
}

static int do_gz_write(void *stream, const void *buf, size_t size)
{
    return gzwrite(stream, buf, size);
}

static char *do_gz_gets(void *stream, char *buf, size_t size)
{
    return gzgets(stream, buf, size);
}


static int do_gz_flush(void *stream)
{
    return gzflush(stream, Z_FULL_FLUSH);
}

static long do_gz_tell(void *stream)
{
    return gztell(stream);
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

static int gz_fseek_wrap(void *stream, long offset, int whence)
{
    z_off_t rc, off = offset;

#if ZLIB_TRACE_SEEK
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
        DBGF_F("%p backward from %lld => %ld (whence %d)\n", stream, gztell(stream), offset, whence);
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

void *do_gz_open(const char *path, const char *mode)
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

void n_stream_zlib_init(tn_stream *st) {
    st->st_open  = do_gz_open;
    st->st_dopen = (void *(*)(int, const char *))gzdopen;
    st->st_read  = do_gz_read;
    st->st_write = do_gz_write;
    st->st_gets  = do_gz_gets;
    st->st_getc  = do_gz_getc;
#if HAVE_GZUNGETC
    st->st_ungetc = do_gz_ungetc;
#else
    st->st_ungetc = NULL;
#endif
    st->st_seek  = gz_fseek_wrap;
    st->st_tell  = do_gz_tell;
    st->st_flush = do_gz_flush;
    st->st_close = (int (*)(void*))gzclose;
}

#endif  /* HAVE_LIBZ */
