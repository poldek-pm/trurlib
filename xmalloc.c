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

static memh mem_handler = NULL;

void (*setxmallocs_handler(void (*handler) (void))) (void) {
    memh tmp = mem_handler;

    mem_handler = handler;

    return tmp;
}


static void nomem(void)
{
    if (mem_handler != NULL)
	mem_handler();
}


void *xmalloc(size_t size)
{
    register void *v;

    if ((v = malloc(size)) == NULL)
	nomem();
    return v;
}


void *xcalloc(size_t nmemb, size_t size)
{
    register void *v = calloc(nmemb, size);
    if (v == 0)
	nomem();
    return v;
}


void *xrealloc(void *ptr, size_t size)
{
    n_assert(size > 0);
    n_assert(ptr != NULL);

    if ((ptr = realloc(ptr, size)) == NULL)
	nomem();

    return ptr;
}


char *xstrdup(const char *s)
{
    register size_t len = strlen(s) + 1;
    register char *new;

    if ((new = xmalloc(len + 1)) == NULL) {
	nomem();
	return NULL;
    }
    return memcpy(new, s, len);
}


void xfree(void *ptr)
{
    n_assert(ptr != NULL);
    free(ptr);
}
