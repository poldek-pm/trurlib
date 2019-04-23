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
#include <zstd.h>

#include "niobuf_int.h"

struct trurl_iobuf {
    struct trurl_io *io;
    void  *iostate;

    int   mode;                 /* TRURL_IO_MODE_* */
    FILE  *stream;
    int   fd;

    off_t pos;
    int   seek;
};

extern struct trurl_io zstdio;

static tn_iobuf *n_iobuf_new(int type, const char *mode)
{
    int m = (*mode == 'r') ? TRURL_IO_MODE_READ : TRURL_IO_MODE_WRITE;
    tn_iobuf *iobuf;

    n_assert(type == TN_STREAM_ZSTDIO); /* zstd is only backed now */

    iobuf = n_calloc(1, sizeof(*iobuf));
    iobuf->mode = m;
    iobuf->io = &zstdio;
    iobuf->iostate = iobuf->io->new(m);

    return iobuf;
}

tn_iobuf *n_iobuf_open(const char *path, const char *mode)
{
    tn_iobuf *iobuf = n_iobuf_new(TN_STREAM_ZSTDIO, mode);
    iobuf->stream = fopen(path, mode);
    iobuf->fd = fileno(iobuf->stream);
    return iobuf;
}

tn_iobuf *n_iobuf_dopen(int fd, const char *mode)
{
    tn_iobuf *iobuf = n_iobuf_new(TN_STREAM_ZSTDIO, mode);
    iobuf->stream = fdopen(fd, mode);
    iobuf->fd = fd;
    return iobuf;
}

int n_iobuf_close(tn_iobuf *iobuf)
{
    iobuf->io->destroy(iobuf->iostate, iobuf->stream);
    iobuf->fd = -1;
    return fclose(iobuf->stream);
}

static long n_iobuf_real_seek(tn_iobuf *iobuf)
{
    if (iobuf->seek < 0) {      /* have to start from beginning */
        int fwd_seek = iobuf->pos + iobuf->seek;
        DBGF("backward %d from %lld => %d\n", iobuf->seek, iobuf->pos, fwd_seek);
        n_assert(fwd_seek >= 0);

        rewind(iobuf->stream);
        iobuf->io->reset(iobuf->iostate);
        iobuf->pos = 0;
        iobuf->seek = fwd_seek; /* processed in next "if" if positive */
    }

    if (iobuf->seek > 0) { /* forward */
        unsigned seek = iobuf->seek;
        uint8_t skipbuf[4096 * 8];

        DBGF("forward %d from %lld\n", iobuf->seek, iobuf->pos);
        while (seek > 0) {
            size_t skipsize = sizeof(skipbuf) > seek ? seek : sizeof(skipbuf);
            n_assert(skipsize > 0);
            int n = iobuf->io->read(iobuf->iostate, iobuf->stream, skipbuf, skipsize);
            if (n == 0)
                return 0;       /* EOF */
            iobuf->pos += n;
            seek -= n;
        }
        iobuf->seek = 0;
    }

    return iobuf->pos;
}

long n_iobuf_seek(tn_iobuf *iobuf, long offset, int whence)
{
    off_t offs = iobuf->pos;

    if (iobuf->seek != 0)       /* seek already requested */
        n_die("n_iobuf: seq seeks are not allowed\n");

    switch (whence) {
        case SEEK_CUR:
            offs = iobuf->pos + offset;
            DBGF("CUR %ld %d\n", offset, iobuf->seek);
            break;

        case SEEK_SET:
            n_assert(offset >= 0);
            DBGF("SET %ld %d\n", offset, iobuf->seek);
            offs = offset;
            break;

        case SEEK_END:
            n_die("iobuf: SEEK_END is not supported\n");
            break;

        default:
            n_die("iobuf: unknown whence (%d)\n", whence);
            break;
    }

    iobuf->seek = offs - iobuf->pos;
    if (iobuf->mode == TRURL_IO_MODE_WRITE && iobuf->seek != 0) {
        n_die("n_iobuf: seek in write mode is not allowed\n");
    }

    return offs;
}

long n_iobuf_tell(tn_iobuf *iobuf) {
    return iobuf->pos + iobuf->seek;
}

int n_iobuf_write(tn_iobuf *iobuf, const void *buf, size_t size)
{
    int n = iobuf->io->write(iobuf->iostate, iobuf->stream, buf, size);

    if (n > 0)
        iobuf->pos += n;

    return n;
}

int n_iobuf_flush(tn_iobuf *iobuf)
{
    int rv = iobuf->io->flush(iobuf->iostate, iobuf->stream);
    int rv2 = fflush(iobuf->stream);
    return rc == 0 && rc2 == 0 ? 0 : EOF;
}

int n_iobuf_read(tn_iobuf *iobuf, void *buf, size_t size)
{
    int n;

    n_iobuf_real_seek(iobuf);   /* pending seek */
    n = iobuf->io->read(iobuf->iostate, iobuf->stream, buf, size);
    iobuf->pos += n;

    return n;
}

static inline
char *n_iobuf_getx(tn_iobuf *iobuf, char *dest, size_t size, int endl)
{
    uint8_t b;
    size_t n;

    n = 0;
    while (n < size && n_iobuf_read(iobuf, &b, 1) == 1) {
        dest[n++] = b;
        if (b == endl)
            break;
    }
    return dest;
}

char *n_iobuf_gets(tn_iobuf *iobuf, char *dest, size_t size)
{
    return n_iobuf_getx(iobuf, dest, size, '\n');
}

int n_iobuf_getc(tn_iobuf *iobuf)
{
    int c = 0;

    if (n_iobuf_read(iobuf, &c, 1) == 1)
        return c;

    return EOF;
}
