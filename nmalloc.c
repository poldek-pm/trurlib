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
#include <stdlib.h>
#include <string.h>

#include "nassert.h"

typedef void (*memh) (void);

static memh mem_fail_fn = NULL;

void (*setxmallocs_handler(void (*handler) (void))) (void) {
    memh tmp = mem_fail_fn;
    mem_fail_fn = handler;
    return tmp;
}

void (*set_nmalloc_fail_hook(void (*fn) (void))) (void) {
    memh tmp = mem_fail_fn;
    mem_fail_fn = fn;
    return tmp;
}


static void nomem(void)
{
    if (mem_fail_fn != NULL)
	mem_fail_fn();
    else {
        printf("Memory exhausted");
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
    memcpy(new, s, length + 1);
    new[length] = '\0';
    return new;
}


char *n_strdup(const char *s)
{
    register size_t len = strlen(s) + 1;
    register char *new;

    if ((new = n_malloc(len + 1)) == NULL) {
	nomem();
	return NULL;
    }
    return memcpy(new, s, len);
}


void n_free(void *ptr)
{
    n_assert(ptr != NULL);
    free(ptr);
}



/* be compatibile with < 0.43.7 */

void *xmalloc(size_t size)
{
    return n_malloc(size);
}

void *xcalloc(size_t nmemb, size_t size)
{
    return n_calloc(nmemb, size);
}

void *xrealloc(void *ptr, size_t size)
{
    return n_realloc(ptr, size);
}

char *xstrdup(const char *s) 
{
    return n_strdup(s);
}

void xfree(void *ptr) 
{
    n_free(ptr);
}

