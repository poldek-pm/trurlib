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

#ifdef USE_XMALLOCS
# include "xmalloc.h"
#endif

#include "trurl_internal.h"
#include "nbuf.h"

struct trurl_buf {
    unsigned char *data;
    size_t        allocated;
    size_t        size;
    size_t        initial_size;
};


tn_buf *n_buf_new(int initial_size)
{
    tn_buf *buf;

    if ((buf = malloc(sizeof(*buf))) == NULL)
	return NULL;

    n_assert(initial_size >= 0);

    if (initial_size < 1)
	initial_size = 2;

    if ((buf->data = malloc(initial_size)) == NULL) {
	free(buf);
	buf = NULL;

    } else {
        buf->data[0] = '\0';
        buf->allocated = initial_size;
        buf->size = 0;
        buf->initial_size = initial_size;
    }

    return buf;
}


tn_buf *n_buf_clean(tn_buf *buf)
{
    buf->size = 0;
    buf->data[0] = '\0';
    return buf;
}


void n_buf_free(tn_buf *buf)
{
    free(buf->data);
    buf->data = NULL;
    free(buf);
}


static tn_buf *n_buf_realloc(tn_buf *buf, size_t new_size)
{
    register int diff;

    diff = new_size - buf->allocated;

    n_assert(diff > 0);

    if (diff > 0) {
	void *tmp;

	if ((tmp = realloc(buf->data, new_size)) == NULL) {
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

    while (req_size >= new_size) {
        new_size += buf->initial_size;
        return n_buf_realloc(buf, new_size);
    }

    return buf;
}


int n_buf_size(const tn_buf *buf)
{
    return buf->size;
}


void *n_buf_dptr(const tn_buf *buf)
{
    return buf->data;
}


void *n_buf_add(tn_buf *buf, const void *data, int size)
{
    register void *p;

    if (buf->allocated - buf->size < (unsigned)size) 
        if (n_buf_grow(buf, buf->allocated + size) == NULL)
	    return NULL;
    
    p = &buf->data[buf->size];
    memcpy(p, data, size);
    buf->size += size;

    return p;
}


char *n_buf_addstring(tn_buf *buf, const char *str, int with_zero)
{
    register void *p;
    register int len;

    len = strlen(str) + 1;

    p = n_buf_add(buf, str, len);
    
    if (p && with_zero) 
        buf->data[buf->size++] = '\0';
    
    return p;
}

