/*
   TRURLib
   Copyright (C) 1999 Pawel A. Gajda (mis@k2.net.pl)

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

#if HAVE_CONFIG_H
# include "config.h"
#endif

#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "nassert.h"
#include "nmalloc.h"

#include "nhash.h"
#include "noash.h"
#include "noash_int.h"
#include "nstr.h"

typedef void (*memh) (void);

static memh mem_fail_fn = NULL;

void (*setxmallocs_handler(void (*handler) (void))) (void) {
    memh tmp = mem_fail_fn;
    mem_fail_fn = handler;
    return tmp;
}

void (*n_malloc_set_failhook(void (*fn) (void)))
{
    memh tmp = mem_fail_fn;
    mem_fail_fn = fn;
    return tmp;
}


static void nomem(void)
{
    if (mem_fail_fn != NULL)
        mem_fail_fn();
    else {
        printf("Memory exhausted\n");
        exit(1);
    }
}


void *n_malloc(size_t size)
{
    register void *v;
    if ((v = malloc(size)) == NULL)
        nomem();
    return v;
}

void *n_calloc(size_t nmemb, size_t size)
{
    register void *v = calloc(nmemb, size);
    if (v == 0)
        nomem();
    return v;
}

void *n_realloc(void *ptr, size_t size)
{
    n_assert(size > 0);
    if ((ptr = realloc(ptr, size)) == NULL)
        nomem();

    return ptr;
}


char *n_strdupl(const char *s, size_t length)
{
    register char *new;

    if ((new = n_malloc(length + 1)) == NULL) {
        nomem();
        return NULL;
    }

    memcpy(new, s, length);
    new[length] = '\0';
    return new;
}


char *n_strdup(const char *s)
{
    register size_t len = strlen(s) + 1;
    register char *new;

    if ((new = n_malloc(len)) == NULL) {
        nomem();
        return NULL;
    }

    return memcpy(new, s, len);
}


void *n_memdup(const void *ptr, size_t size)
{
    register void *new;

    if ((new = n_malloc(size)) == NULL) {
        nomem();
        return NULL;
    }

    return memcpy(new, ptr, size);
}

void n_free(void *ptr)
{
    free(ptr);
}

void n_cfree(void *ptr)
{
    void **pptr = (void**)ptr;

    if (*pptr) {
        free(*pptr);
        *pptr = NULL;
    }
}

static void *malloc_malloc(tn_alloc *na, size_t size)
{
    na = na;
    return n_malloc(size);
}

static
void *malloc_calloc(tn_alloc *na, size_t size)
{
    na = na;
    return n_calloc(size, 1);
}

static
void *malloc_realloc(tn_alloc *na, void *ptr, size_t size, size_t newsize)
{
    na = na; size = size;
    return n_realloc(ptr, newsize);
}


static
void malloc_free(tn_alloc *na, void *ptr)
{
    na = na;
    return n_free(ptr);
}

#define obstack_chunk_alloc n_malloc
#define obstack_chunk_free  free
#if HAVE_OBSTACK
# include <obstack.h>
#else
# include "lib/obstack.h"
#endif

struct privdata {
    struct obstack ob;
    tn_oash  *ht;
};

static
void *aobstack_malloc(tn_alloc *na, size_t size)
{
    struct privdata *pd = na->_privdata;
    return obstack_alloc(&pd->ob, size);
}

static
void *aobstack_calloc(tn_alloc *na, size_t size)
{
    struct privdata *pd = na->_privdata;
    void *ptr = obstack_alloc(&pd->ob, size);
    memset(ptr, 0, size);
    return ptr;
}

static
void aobstack_free(tn_alloc *na, void *ptr)
{
    (void)na;
    (void)ptr;
}

void *aobstack_realloc(tn_alloc *na, void *ptr, size_t size, size_t newsize)
{
    struct privdata *pd = na->_privdata;
    void *new;

    new = obstack_alloc(&pd->ob, newsize);
    memcpy(new, ptr, size);
    return new;
}

static
const void *aobstack_alloc_str(tn_alloc *na, uint32_t max, const char *str, size_t len)
{
    struct privdata *pd = na->_privdata;

    n_assert(len > 0);
    n_assert(len <= UINT16_MAX);

    if (pd->ht == NULL) {
        int slots = pd->ob.chunk_size; /* TODO: make initial slots configurable  */
        pd->ht = n_oash_new_na(na, slots, NULL);
        n_oash_ctl(pd->ht, TN_HASH_NOCPKEY | TN_HASH_REHASH);
    }

    struct dentry *de = n_oash__get_insert(pd->ht, str, len);
    n_assert(de);

    if (de->data == NULL) {
        tn_str8 *s8 = NULL;
        tn_str16 *s16 = NULL;

        switch (max) {
        case UINT8_MAX:
            s8 = na->na_malloc(na, sizeof(*s8) + len + 1);
            s8->len = len;
            memcpy(s8->str, str, len);
            s8->str[len] = '\0';
            de->key = s8->str;
            de->data = s8;
            break;

        case UINT16_MAX:
            s16 = na->na_malloc(na, sizeof(*s16) + len + 1);
            s16->len = len;
            memcpy(s16->str, str, len);
            s16->str[len] = '\0';

            de->key = s16->str;
            de->data = s16;
            break;

        default:
            n_die("%u: invalid max arg\n", max);
            break;
        }
    }

    return de->data;
}

const tn_str8 *aobstack_alloc_str8(tn_alloc *na, const char *str, size_t len)
{
    return aobstack_alloc_str(na, UINT8_MAX, str, len);
}

const tn_str16 *aobstack_alloc_str16(tn_alloc *na, const char *str, size_t len)
{
    return aobstack_alloc_str(na, UINT16_MAX, str, len);
}

void n_alloc_debug_dump(tn_alloc *na)
{
    struct privdata *pd = na->_privdata;
    fprintf(stderr, "%p chunk_size %ld\n", na, pd->ob.chunk_size);
}

tn_alloc *n_alloc_new(size_t chunkkb, unsigned int flags)
{
    tn_alloc *na;

    na = n_calloc(1, sizeof(*na));
    na->_refcnt = 0;
    na->_flags = flags;

    if (flags & TN_ALLOC_MALLOC) {
        na->_privdata = NULL;
        na->na_malloc = malloc_malloc;
        na->na_calloc = malloc_calloc;
        na->na_free   = malloc_free;
        na->na_realloc = malloc_realloc;

    } else if (flags & TN_ALLOC_OBSTACK) {
        struct privdata *pd = n_calloc(1, sizeof(*pd));
        //struct obstack *ob = n_malloc(sizeof(*ob));
        obstack_init(&pd->ob);
        if (chunkkb < 2)
            chunkkb = 2;

        if (chunkkb > 4096)
            fprintf(stderr, "n_alloc_new: do you really request obstack's "
                    "chunk size greater than 4M?\n");

        obstack_chunk_size(&pd->ob) = 1024 * chunkkb;
#if HAVE_CPU_UNALIGNED_ACCESS
        obstack_alignment_mask(&pd->ob) = 0; /* TODO: configurable */
#endif
        na->_privdata = pd;
        na->na_malloc  = aobstack_malloc;
        na->na_calloc  = aobstack_calloc;
        na->na_free    = aobstack_free;
        na->na_realloc = aobstack_realloc;
        na->na_alloc_str8 = aobstack_alloc_str8;
        na->na_alloc_str16 = aobstack_alloc_str16;

    } else {
        n_assert(0);
    }


    return na;
}

void n_alloc_free(tn_alloc *na)
{
    struct privdata *pd = na->_privdata;

    /* egg&chicken - "internal" ref made by pd->ht, so let it call n_alloc_free()
       again but clear pd->ht pointer before to avoid loop */
    if (na->_refcnt == 1 && (na->_flags & TN_ALLOC_OBSTACK) && pd->ht) {
        tn_oash *ht = pd->ht;
        pd->ht = NULL;
        n_oash_free(ht);
    }

    if (na->_refcnt > 0) {
        na->_refcnt--;
        return;
    }

    if (na->_flags & TN_ALLOC_OBSTACK) {
        n_assert(pd->ht == NULL);
        obstack_free(&pd->ob, NULL);
        n_free(pd);
    }

    n_free(na);
}
