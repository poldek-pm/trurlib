/* 
   TRURLib
   Copyright (C) 1999, 2000 Pawel A. Gajda (mis@k2.net.pl)

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

#include <stdarg.h>
#include <stdlib.h>
#include <string.h>

#include "trurl_internal.h"
#include "nlist.h"
#include "nmalloc.h"

struct list_node {
    void *data;
    struct list_node *next;
};

struct trurl_list {
    uint16_t    _refcnt;
    uint16_t    flags;

    int items;

    struct list_node *head;
    struct list_node *tail;

    t_fn_free free_fn;
    t_fn_cmp cmp_fn;
};

