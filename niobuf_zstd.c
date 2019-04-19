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

struct zstd_state {
    int            mode;
    void           *stream;

    ZSTD_inBuffer  inbuf;
    ZSTD_outBuffer outbuf;

    size_t         bufsize;
    void          *bufdata;
};

static ZSTD_CStream *zstd_new_c_stream(size_t *size) {
    ZSTD_CStream *stream;

    if ((stream = ZSTD_createCStream()) == NULL)
        return NULL;

    int rc = ZSTD_initCStream(stream, 3);

    if (ZSTD_isError(rc)) {
        ZSTD_freeCStream(stream);
        return NULL;
    }

    *size = ZSTD_CStreamOutSize();
    return stream;
}

static ZSTD_DStream *zstd_new_d_stream(size_t *size) {
    ZSTD_DStream *stream;

    if ((stream = ZSTD_createDStream()) == NULL)
        return NULL;

    int rc = ZSTD_initDStream(stream);

    if (ZSTD_isError(rc)) {
        ZSTD_freeDStream(stream);
        return NULL;
    }

    *size = ZSTD_DStreamInSize();
    return stream;
}

static struct zstd_state *zstd_new(int mode) {
    void *stream;
    size_t size;

    n_assert(mode == TRURL_IO_MODE_READ || mode == TRURL_IO_MODE_WRITE);

    if (mode == TRURL_IO_MODE_READ) {
        stream = zstd_new_d_stream(&size);
    } else {
        stream = zstd_new_c_stream(&size);
    }
    DBGF("stream %p\n", stream);
    if (stream == NULL)
        return NULL;

    struct zstd_state *st = n_calloc(1, sizeof(*st));
    st->mode = mode;
    st->stream = stream;
    st->bufsize = size;
    st->bufdata = n_malloc(size);
    //DBGF("zstd_new %p %p %d\n", stream, st->bufdata, size);

    return st;
}

static void zstd_reset(struct zstd_state *st)
{
    size_t size;

    /*
    // reset*Stream is experimental API
    if (st->mode == TRURL_IO_MODE_READ)
        ZSTD_resetDStream(st->stream);
    else
        ZSTD_resetCStream(st->stream);
    */

    if (st->mode == TRURL_IO_MODE_READ) {
        ZSTD_freeDStream(st->stream);
        st->stream = zstd_new_d_stream(&size);
    } else {
        ZSTD_freeCStream(st->stream);
        st->stream = zstd_new_c_stream(&size);
    }

    st->inbuf.pos = st->inbuf.size = 0;
    st->outbuf.pos = st->outbuf.size = 0;
}

static int fill_inbuf(struct zstd_state *st, FILE *file)
{
    n_assert(st->inbuf.pos == st->inbuf.size);

    st->inbuf.size = fread(st->bufdata, 1, st->bufsize, file);
    st->inbuf.src = st->bufdata;
    st->inbuf.pos = 0;

    return st->inbuf.size;
}


static
int zstd_read(struct zstd_state *st, FILE *file, void *dest, size_t dsize)
{
    ZSTD_outBuffer output = { dest, dsize, 0 };

    //DBGF("read.start %d %d\n", st->inbuf.pos, st->inbuf.size);
    while (output.pos < output.size) {
        //DBGF("read %d %d\n", st->inbuf.pos, st->inbuf.size);
        n_assert(st->inbuf.pos <= st->inbuf.size);

        if (st->inbuf.pos == st->inbuf.size) { /* empty or consumed */
            if (fill_inbuf(st, file) == 0)     /* EOF */
                break;
        }

	int n = ZSTD_decompressStream(st->stream, &output, &st->inbuf);
	if (ZSTD_isError(n)) {
            fprintf(stderr, "ZSTD_decompressStream: %s", ZSTD_getErrorName(n));
	    return -1;
	}
    }

    return output.pos;
}

static void reset_outbuf(struct zstd_state *st)
{
    st->outbuf.dst = st->bufdata;
    st->outbuf.size = st->bufsize;
    st->outbuf.pos = 0;
}

static int flush_outbuf(struct zstd_state *st, FILE *file)
{
    int nw = 0;

    if (st->outbuf.pos > 0) {
        size_t n = fwrite(st->bufdata, 1, st->outbuf.pos, file);
        if (n != st->outbuf.pos) {
            fprintf(stderr, "ZSTD write decompressed data failed");
            return -1;
        }
        nw += n;
    }

    return nw;
}

static
int zstd_write(struct zstd_state *st, FILE *file, const void *src, size_t ssize) {
    ZSTD_inBuffer input = { src, ssize, 0 };
    int nw = 0;

    while (input.pos < input.size) {
        reset_outbuf(st);

        int n = ZSTD_compressStream(st->stream, &st->outbuf, &input);
        if (ZSTD_isError(n)) {
            fprintf(stderr, "ZSTD_compressStream: %s", ZSTD_getErrorName(n));
	    return -1;
	}

        if ((n = flush_outbuf(st, file)) == -1)
            return -1;

        nw += n;
    }

    return input.pos;
}


static int zstd_flush(struct zstd_state *st, FILE *file)
{
    if (st->mode == TRURL_IO_MODE_READ)
	return 0;

    int n = 0, nw = 0;
    do {
        reset_outbuf(st);

        n = ZSTD_flushStream(st->stream, &st->outbuf);
        if (ZSTD_isError(n)) {
            fprintf(stderr, "ZSTD_flushStream: %s", ZSTD_getErrorName(n));
            return -1;
        }

        int nf;

        if ((nf = flush_outbuf(st, file)) == -1)
            return -1;

        nw += nf;

    } while (n > 0);

    return nw;
}

static void zstd_free(struct zstd_state *st)
{
    if (st->mode == TRURL_IO_MODE_READ) {
	ZSTD_freeDStream(st->stream);
    } else {
        ZSTD_freeCStream(st->stream);
    }

    if (st->bufdata)
        n_free(st->bufdata);

    n_cfree(&st);
}

static int zstd_destroy(struct zstd_state *st, FILE *file)
{
    if (st->mode == TRURL_IO_MODE_READ) {
        zstd_free(st);
        return 0;
    }

    zstd_flush(st, file);
    reset_outbuf(st);

    int n = ZSTD_endStream(st->stream, &st->outbuf);
    if (ZSTD_isError(n)) {
        fprintf(stderr, "ZSTD_endStream: %s", ZSTD_getErrorName(n));
        zstd_free(st);
        return -1;
    }

    if (flush_outbuf(st, file) == -1)
        return -1;

    zstd_free(st);
    return 0;
}

struct trurl_io zstdio = {
    "zstdio",
    (void *(*)(int)) zstd_new,
    (int (*)(void *, FILE *, void *, size_t))zstd_read,
    (int   (*)(void *, FILE *, const void *, size_t)) zstd_write,
    (int   (*)(void *, FILE *)) zstd_flush,
    (void  (*)(void *)) zstd_reset,
    (int   (*)(void*, FILE *)) zstd_destroy
};
