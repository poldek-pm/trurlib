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

/*
   $Id$
 */


#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "nassert.h"
#include "nmalloc.h"

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
    if (ptr)
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
    return n_malloc(size);
}

static
void *malloc_calloc(tn_alloc *na, size_t size)
{
    return n_calloc(size, 1);
}

static
void *malloc_realloc(tn_alloc *na, void *ptr, size_t size, size_t newsize)
{
    return n_realloc(ptr, newsize);
}


static
void malloc_free(tn_alloc *na, void *ptr)
{
    return n_free(ptr);
}



#define obstack_chunk_alloc n_malloc
#define obstack_chunk_free  n_free
#include <obstack.h>

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
    ptr = ptr;
}


void *aobstack_realloc(tn_alloc *na, void *ptr, size_t size, size_t newsize)
{
    void *new;
    new = obstack_alloc((struct obstack*)na->_privdata, newsize);
    memcpy(new, ptr, size);
    return new;
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
        if (chunkkb < 4) 
            chunkkb = 4;
        obstack_chunk_size(ob) = 1024 * chunkkb;
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

