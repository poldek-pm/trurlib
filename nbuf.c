/*
   TRURLib dynamic buffer

   Copyright (C) 2000 Pawel A. Gajda (mis@k2.net.pl)

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public
   License along with this library; if not, write to the
   Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.
 */

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "nassert.h"
#include "trurl_internal.h"
#include "nmalloc.h"
#include "nbuf.h"
#include "n_snprintf.h"
#include "nstream.h"
#include "nstore.h"
#include "ndie.h"


tn_buf *n_buf_new(int initial_size)
{
    tn_buf *buf;

    if ((buf = n_calloc(1, sizeof(*buf))) == NULL)
        return NULL;

    if (initial_size == 0) {
        buf->data = NULL;
        return buf;
    }

    buf->data = NULL;
    buf->allocated = initial_size;
    buf->data = n_malloc(initial_size);
    buf->data[0] = '\0';
    return buf;
}


tn_buf *n_buf_init(tn_buf *buf, void *buffer, int size)
{
    n_assert(buf->data == NULL);
    buf->off = 0;
    buf->data = buffer;
    buf->allocated = size;
    buf->size = size;
    buf->flags = TN_BUF_CONSTSIZE | TN_BUF_CONSTDATA;
    return buf;
}


tn_buf *n_buf_ctl(tn_buf *buf, unsigned flags)
{
    buf->flags |= flags;
    return buf;
}


tn_buf *n_buf_clean(tn_buf *buf)
{
    if (buf->flags & TN_BUF_CONSTDATA) {
        buf->data = NULL;
        buf->allocated = 0;
        buf->flags = 0;
        buf->off  = 0;

    } else {
        buf->size = 0;
        buf->off  = 0;
        buf->data[0] = '\0';
    }

    return buf;
}


void n_buf_free(tn_buf *buf)
{
    if (buf->_refcnt > 0) {
        buf->_refcnt--;
        return;
    }

    if (!(buf->flags & TN_BUF_CONSTDATA))
        free(buf->data);
    buf->data = NULL;
    free(buf);
}


static tn_buf *n_buf_realloc(tn_buf *buf, size_t new_size)
{
    register int diff;

    diff = new_size - buf->allocated;

    n_assert(diff > 0);

    if (buf->flags & TN_BUF_CONSTSIZE) {
        trurl_die("n_buf_grow: grow request for const size buffer");
        return NULL;
    }

    if (diff > 0) {
        void *tmp;

        DBGF("%p to %d: allocated %d, size %d, off %d\n", new_size,
             buf, buf->allocated, buf->size, buf->off);

        if ((tmp = n_realloc(buf->data, new_size)) == NULL) {
            return NULL;

        } else {
            buf->data = tmp;
            buf->allocated = new_size;
        }
    }
    return buf;
}


static tn_buf *n_buf_grow(tn_buf *buf, size_t req_size)
{
    register size_t new_size = buf->allocated;

    if (new_size == 0)
        new_size = 2;

    while (req_size >= new_size)
        new_size *= 2;

    return n_buf_realloc(buf, new_size);
}

#if 0
int n_buf_size(const tn_buf *buf)
{
    return buf->size;
}

void *n_buf_ptr(const tn_buf *buf)
{
    return buf->data;
}
#endif

int n_buf_seek(tn_buf *buf, int off, int whence)
{
    switch (whence) {
        case SEEK_CUR:
            buf->off += off;
            break;

        case SEEK_SET:
            buf->off = off;
            break;

        case SEEK_END:
            buf->off = buf->size;
            if (off != 0)
                n_die("nbuf: offset must be 0 with SEEK_END\n");
            break;

        default:
            n_die("nbuf: unknown whence (%d)\n", whence);
            break;
    }

    return buf->off;
}




int n_buf_write_ex(tn_buf *buf, const void *ptr, int size, int zero)
{
    if (zero)
        zero = 1;

    /* size + zero + 1 - gets() must guarantee that buf is longer
       than returned string */
    if (buf->allocated - buf->off < (unsigned)size + zero + 1)
        if (n_buf_grow(buf, buf->allocated + size + zero + 1) == NULL)
            return -1;

    memcpy(&buf->data[buf->off], ptr, size);
    buf->off += size;

    if (buf->off > buf->size)
        buf->size = buf->off;

    if (zero)
        buf->data[buf->off] = '\0';

    return size;
}

void n_buf_it_init(tn_buf_it *bufi, tn_buf *buf)
{
    bufi->nbuf = buf;
    bufi->offs = 0;
}

#if 0
void *n_buf_it_get(tn_buf_it *bufi, size_t size)
{
    unsigned char *ptr;
    register int boff = bufi->offs;

    if (boff + size > bufi->nbuf->size)
        return NULL;

    ptr = &bufi->nbuf->data[boff];
    boff += size;
    bufi->offs = boff;
    return ptr;
}
#endif

char *n_buf_it_gets_ext(tn_buf_it *bufi, size_t *len, int endl)
{
    unsigned char *ptr;
    tn_buf *buf;


    buf = bufi->nbuf;

    if (bufi->offs + 1 > buf->size)
        return NULL;

    ptr = &buf->data[bufi->offs];
    *len = bufi->offs;

    while (bufi->offs < buf->size && buf->data[bufi->offs] != endl)
        bufi->offs++;

    *len = bufi->offs - *len;
    bufi->offs++;               /* skip endl */
    return (char *)ptr;
}


#undef n_buf_add
// legacy: provide n_buf_add symbol
int n_buf_add(tn_buf *buf, const void *data, int size)
{
    return n_buf_write_ex(buf, data, size, 0);
}

#undef n_buf_addata
// legacy: provide n_buf_add symbol
int n_buf_addata(tn_buf *buf, const void *data, int size, int zero)
{
    return n_buf_write_ex(buf, data, size, zero);
}

#undef n_buf_addstring
// legacy: provide n_buf_add symbol
int n_buf_addstring(tn_buf *buf, const char *str, int zero)
{
    return n_buf_write_ex(buf, str, strlen(str), zero);
}


int n_buf_vprintf(tn_buf *nbuf, const char *fmt, va_list args)
{
    char     buf[4096];
    int      n;

    n = n_vsnprintf(buf, sizeof(buf), fmt, args);
    n_buf_write_ex(nbuf, buf, n, 1);
    //n_buf_add(nbuf, buf, n);
    return n;
}


int n_buf_printf(tn_buf *nbuf, const char *fmt, ...)
{
    va_list  args;
    int n;

    va_start(args, fmt);
    n = n_buf_vprintf(nbuf, fmt, args);
    va_end(args);

    return n;
}


#include <netinet/in.h>
#define hton16(v)  htons(v)
#define hton32(v)  htonl(v)

#define ntoh16(v)  ntohs(v)
#define ntoh32(v)  ntohl(v)



int n_buf_store(tn_buf *nbuf, tn_stream *st, int sizebits)
{
    if (sizebits == 0)
        n_store_uint32(st, n_buf_size(nbuf));

    else {
        switch (sizebits) {
            case TN_BUF_STORE_8B:
                n_assert(0);
                break;

            case TN_BUF_STORE_16B:
                n_assert(n_buf_size(nbuf) < INT16_MAX);
                n_stream_write_uint16(st, n_buf_size(nbuf));
                break;

            case TN_BUF_STORE_32B:
                n_assert(n_buf_size(nbuf) < INT32_MAX);
                n_stream_write_uint32(st, n_buf_size(nbuf));
                break;
        }
    }


    return n_stream_write(st, n_buf_ptr(nbuf), n_buf_size(nbuf)) ==
        n_buf_size(nbuf);
}


int n_buf_store_buf(tn_buf *nbuf, tn_buf *tonbuf, int sizebits)
{
    if (sizebits == 0)
        n_assert(0);
    //n_store_uint32(st, n_buf_size(nbuf));

    else {
        switch (sizebits) {
            case TN_BUF_STORE_8B:
                n_assert(0);
                break;

            case TN_BUF_STORE_16B:
                n_assert(n_buf_size(nbuf) < INT16_MAX);
                n_buf_add_int16(tonbuf, n_buf_size(nbuf));
                break;

            case TN_BUF_STORE_32B:
                n_assert(n_buf_size(nbuf) < INT32_MAX);
                n_buf_add_int32(tonbuf, n_buf_size(nbuf));
                break;
        }
    }

    return n_buf_write(tonbuf, n_buf_ptr(nbuf), n_buf_size(nbuf));
}



int n_buf_restore_skip(tn_stream *st, int sizebits)
{
    uint32_t size;

    if (sizebits == 0)
        n_restore_uint32(st, &size);

    else {
        switch (sizebits) {
            case TN_BUF_STORE_8B:
            {
                uint8_t v = 0;
                n_stream_read_uint8(st, &v);
                size = v;
                break;
            }

            case TN_BUF_STORE_16B:
            {
                uint16_t v = 0;
                n_stream_read_uint16(st, &v);
                size = v;
                break;
            }


            case TN_BUF_STORE_32B:
                n_stream_read_uint32(st, &size);
                break;

            default:
                n_assert(0);
        }
    }


    return n_stream_seek(st, size, SEEK_CUR);
}



int n_buf_restore_ex(tn_stream *st, tn_buf **bptr, int sizebits,
                     int (*process_buf)(tn_buf *, void *), void *arg)
{
    tn_buf         *nbuf;
    char           *buf;
    uint32_t       size;
    int            rc = 0;


    if (bptr)
        *bptr = NULL;

    if (sizebits == 0)
        n_restore_uint32(st, &size);

    else {
        switch (sizebits) {
            case TN_BUF_STORE_8B: {
                uint8_t v = 0;
                n_stream_read_uint8(st, &v);
                size = v;
                break;
            }

            case TN_BUF_STORE_16B:
            {
                uint16_t v = 0;
                n_stream_read_uint16(st, &v);
                size = v;
                break;
            }


            case TN_BUF_STORE_32B:
                n_stream_read_uint32(st, &size);
                break;

            default:
                n_assert(0);
        }
    }

    //printf("n_buf_restore %d, %lu\n", size, n_stream_tell(st));
    if (process_buf == NULL) {
        buf = n_malloc(size);
    } else {
        buf = alloca(size);
    }

    if (n_stream_read(st, buf, size) != (int)size) {
        if (process_buf == NULL)
            free(buf);
        return 0;
    }

    nbuf = n_buf_new(0);
    n_buf_init(nbuf, buf, size);

    if (process_buf == NULL) {
        nbuf->flags &= ~(TN_BUF_CONSTDATA); /* managed by buffer */
        *bptr = nbuf;
        rc = 1;

    } else {
        rc = process_buf(nbuf, arg);
        n_buf_free(nbuf);
    }

    return rc;
}
