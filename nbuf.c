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

/*
   $Id$
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#ifdef USE_N_ASSERT
# include "nassert.h"
#else
# include <assert.h>
# define n_assert(expr) assert(expr)
#endif

#include "trurl_internal.h"
#include "nmalloc.h"
#include "nbuf.h"
#include "n_snprintf.h"
#include "nstream.h"
#include "nstore.h"

struct trurl_buf {
    unsigned char *data;
    size_t        allocated;
    size_t        size;
    unsigned int  flags;
};


tn_buf *n_buf_new(int initial_size)
{
    tn_buf *buf;

    if ((buf = n_malloc(sizeof(*buf))) == NULL)
	return NULL;

    n_assert(initial_size >= 0);
    
    buf->allocated = initial_size;
    buf->size = 0;
    buf->flags = 0;
    
    if (initial_size == 0) {
        buf->data = NULL;
        
    } else if ((buf->data = n_malloc(initial_size)) != NULL) {
        buf->data[0] = '\0';

    } else {
        free(buf);
        buf = NULL;
    }
    
    return buf;
}


tn_buf *n_buf_init(tn_buf *buf, void *buffer, int size)
{
    n_assert(buf->data == NULL);
    
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
    if (!(buf->flags & TN_BUF_CONSTDATA)) {
        buf->size = 0;
        buf->data[0] = '\0';
    }
    return buf;
}


void n_buf_free(tn_buf *buf)
{
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


int n_buf_size(const tn_buf *buf)
{
    return buf->size;
}


void *n_buf_ptr(const tn_buf *buf)
{
    return buf->data;
}


int n_buf_addata(tn_buf *buf, const void *data, int size, int zero)
{
    int offs;

    if (zero)
        zero = 1;
    
    if (buf->allocated - buf->size < (unsigned)size + zero) 
        if (n_buf_grow(buf, buf->allocated + size + zero) == NULL)
	    return -1;

    memcpy(&buf->data[buf->size], data, size);
    offs = buf->size;
    buf->size += size;
    if (zero)
        buf->data[buf->size] = '\0';
    
    return offs;
}


int n_buf_addstring(tn_buf *buf, const char *str, int with_zero)
{
    register int len;

    len = strlen(str);
    
    if (with_zero)
        len++;
    
    return n_buf_add(buf, str, len);
}

void n_buf_it_init(tn_buf_it *bufi, tn_buf *buf) 
{
    bufi->nbuf = buf;
    bufi->offs = 0;
}

void *n_buf_it_get(tn_buf_it *bufi, size_t size) 
{
    unsigned char *ptr;
    
    if (bufi->offs + size > bufi->nbuf->size)
        return NULL;

    ptr = &bufi->nbuf->data[bufi->offs];
    bufi->offs += size;
    return ptr;
}


#undef n_buf_add
// legacy: provide n_buf_add symbol 
int n_buf_add(tn_buf *buf, const void *data, int size) 
{
    return n_buf_addata(buf, data, size, 0);
}


int n_buf_vprintf(tn_buf *nbuf, const char *fmt, va_list args)
{
    char     buf[4096];
    int      n;
    
    n = n_vsnprintf(buf, sizeof(buf), fmt, args);
    n_buf_add(nbuf, buf, n);
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
                n_assert(n_buf_size(nbuf) < UINT16_MAX);
                n_stream_write_uint16(st, n_buf_size(nbuf));
                break;

            case TN_BUF_STORE_32B:
                n_assert(n_buf_size(nbuf) < UINT32_MAX);
                n_stream_write_uint32(st, n_buf_size(nbuf));
                break;
        }
    }
    
        
    return n_stream_write(st, n_buf_ptr(nbuf), n_buf_size(nbuf)) ==
        n_buf_size(nbuf);
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
                uint8_t v;
                n_stream_read_uint8(st, &v);
                size = v;
                break;
            }
            
            case TN_BUF_STORE_16B:
            {
                uint16_t v;
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

    n_stream_seek(st, size, SEEK_CUR);
}



int n_buf_restore_ex(tn_stream *st, tn_buf **bptr, int sizebits, 
                     int (*process_buf)(tn_buf *, void *), void *arg) 
{
    tn_buf         *nbuf;
    char           *buf;
    uint32_t       size;
    uint8_t        flag;
    int            rc = 0;


    if (bptr)
        *bptr = NULL;

    if (sizebits == 0)
        n_restore_uint32(st, &size);
    
    else {
        switch (sizebits) {
            case TN_BUF_STORE_8B: {
                uint8_t v;
                n_stream_read_uint8(st, &v);
                size = v;
                break;
            }
                
            case TN_BUF_STORE_16B: 
            {
                uint16_t v;
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
    
    buf = alloca(size);
    if (n_stream_read(st, buf, size) != size)
        return 0;

    if (process_buf == NULL) {
        nbuf = n_buf_new(size);
        n_buf_addata(nbuf, buf, size, 0);
        *bptr = nbuf;
        rc = 1;
        
    } else {
        nbuf = n_buf_new(0);
        n_buf_init(nbuf, buf, size);
        rc = process_buf(nbuf, arg);
        n_buf_free(nbuf);
    }
        
    return rc;
}


