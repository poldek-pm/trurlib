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

static
void *aobstack_malloc(tn_alloc *na, size_t size)
{
    return obstack_alloc((struct obstack*)na->_privdata, size);
}

static
void *aobstack_calloc(tn_alloc *na, size_t size)
{
    void *ptr = obstack_alloc((struct obstack*)na->_privdata, size);
    memset(ptr, 0, size);
    return ptr;
}


static
void aobstack_free(tn_alloc *na, void *ptr)
{
    na = na;
    ptr = ptr;
}


void *aobstack_realloc(tn_alloc *na, void *ptr, size_t size, size_t newsize)
{
    void *new;
    new = obstack_alloc((struct obstack*)na->_privdata, newsize);
    memcpy(new, ptr, size);
    return new;
}

void n_alloc_debug_dump(tn_alloc *na)
{
    fprintf(stderr, "%p chunk_size %ld\n", na,
            ((struct obstack*)na->_privdata)->chunk_size);
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
        struct obstack *ob = n_malloc(sizeof(*ob));
        obstack_init(ob);
        if (chunkkb < 2)
            chunkkb = 2;

        if (chunkkb > 4096)
            fprintf(stderr, "n_alloc_new: do you really request obstack's "
                    "chunk size greater than 4M?\n");


        obstack_chunk_size(ob) = 1024 * chunkkb;
#if HAVE_CPU_UNALIGNED_ACCESS
        obstack_alignment_mask(ob) = 0; /* TODO: configurable */
#endif
        na->_privdata = ob;
        na->na_malloc  = aobstack_malloc;
        na->na_calloc  = aobstack_calloc;
        na->na_free    = aobstack_free;
        na->na_realloc = aobstack_realloc;

    } else {
        n_assert(0);
    }


    return na;
}

void n_alloc_free(tn_alloc *na)
{
    if (na->_refcnt > 0) {
        na->_refcnt--;
        return;
    }

    if (na->_flags & TN_ALLOC_OBSTACK) {
        obstack_free(na->_privdata, NULL);
        n_free(na->_privdata);
    }

    n_free(na);
}

/* deduplicate string  allocator */
struct trurl_strdalloc_private {
    tn_hash  *ht;
    tn_alloc *na;
    int      hits;
};

tn_strdalloc *n_strdalloc_new(size_t initial_slots, int flags)
{
    flags = flags;                      /* unused */
    size_t kbsize = (initial_slots * 32 /* avg str len */) / 1024;

    tn_alloc *na = n_alloc_new(kbsize, TN_ALLOC_OBSTACK);
    tn_strdalloc *sa = na->na_malloc(na, sizeof(*sa));

    sa->ht = n_hash_new_na(na, initial_slots, NULL);
    sa->na = na;
    sa->hits = 0;

    n_hash_ctl(sa->ht, TN_HASH_NOCPKEY | TN_HASH_REHASH);
    return sa;
}

void n_strdalloc_free(tn_strdalloc *sa)
{
    tn_alloc *na = sa->na;
    n_hash_free(sa->ht);
    memset(sa, '0', sizeof(*sa));
    n_alloc_free(na);
}

static
const void *do_strdalloc_add(tn_strdalloc *sa, uint32_t max, const char *str, size_t len)
{
    void *ent;

    n_assert(len > 0);
    n_assert(len <= max);

    uint32_t khash = n_hash_compute_hash(sa->ht, str, len);

    if ((ent = n_hash_hget(sa->ht, str, len, khash))) {
        sa->hits++;
        return ent;
    }

    char *entstr = NULL;
    tn_lstr8 *ent8 = NULL;
    tn_lstr16 *ent16 = NULL;
    switch (max) {
        case UINT8_MAX:
            ent8 = sa->na->na_malloc(sa->na, sizeof(*ent8) + len + 1);
            ent8->len = len;
            entstr = ent8->str;
            ent = ent8;
            break;

        case UINT16_MAX:
            ent16 = sa->na->na_malloc(sa->na, sizeof(*ent16) + len + 1);
            ent16->len = len;
            entstr = ent16->str;
            ent = ent16;
            break;

        default:
            n_die("%u: invalid max arg\n", max);
            break;
    }

    n_assert(entstr);
    n_strncpy(entstr, str, len + 1);
    n_hash_hinsert(sa->ht, entstr, len, khash, ent);

    return ent;
}

const tn_lstr8 *n_strdalloc_add8(tn_strdalloc *sa, const char *str, size_t len)
{
    return do_strdalloc_add(sa, UINT8_MAX, str, len);
}

const tn_lstr16 *n_strdalloc_add16(tn_strdalloc *sa, const char *str, size_t len)
{
    return do_strdalloc_add(sa, UINT16_MAX, str, len);
}
