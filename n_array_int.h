#ifndef TRURL_NARRAY_INT_H
#define TRURL_NARRAY_INT_H
/* 
   TRURLib dynamic array of void pointers 
   
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
#include "narray.h"
#include "nmalloc.h"
#include "tfn_types.h"

#define ARR_SORTED        (1 << 8)

#define SETARR_SORTED(arr)   ((arr)->flags |= ARR_SORTED)
#define SETARR_UNSORTED(arr) ((arr)->flags &= ~ARR_SORTED) 
#define ISARR_SORTED(arr)    ((arr)->flags & ARR_SORTED)

struct trurl_array {
    size_t      items;          
    void        **data;
    size_t      allocated;
    unsigned    flags;
    size_t      incstep;
    size_t      start_index;

    t_fn_free   free_fn;
    t_fn_cmp    cmp_fn;
};


tn_array *n_array_grow_priv_(tn_array *arr, size_t req_size);


#endif /* TRURL_NARRAY_INT_H */
