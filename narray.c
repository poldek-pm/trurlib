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

#ifdef USE_XMALLOCS
# include "xmalloc.h"
#endif

#include "trurl_internal.h"
#include "narray.h"
#include "tfn_types.h"

#ifndef MODULES
# define MODULE_n_array_new
# define MODULE_n_array_ctl_growth
# define MODULE_n_array_clean
# define MODULE_n_array_free
# define MODULE_n_array_grow_priv
# define MODULE_n_array_nth
# define MODULE_n_array_remove_nth
# define MODULE_n_array_set_nth
# define MODULE_n_array_push
# define MODULE_n_array_pop
# define MODULE_n_array_shift
# define MODULE_n_array_unshift
# define MODULE_n_array_uniq_ex
# define MODULE_n_array_eq_ex
# define MODULE_n_array_dup
# define MODULE_n_array_sorts
# define MODULE_n_array_bsearch_ex
# define MODULE_n_array_map
# define MODULE_n_array_map_arg
# define MODULE_n_array_dump_stats
#endif


struct trurl_array {
    size_t      items;          
    void        **data;
    size_t      allocated;
    int         inctype;
    size_t      incstep;
    size_t      start_index;

    t_fn_free   free_fn;
    t_fn_cmp    cmp_fn;
};

tn_array *n_array_grow_priv_(tn_array *arr, size_t req_size);


#ifdef MODULE_n_array_new
tn_array *n_array_new(int initial_size, t_fn_free freef, t_fn_cmp cmpf)
{
    tn_array *arr;

    if ((arr = calloc(1, sizeof(*arr))) == NULL)
	return NULL;

    n_assert(initial_size >= 0);

    if (initial_size < 1)
	initial_size = 2;

    if ((arr->data = malloc(initial_size * sizeof(*arr->data))) == NULL) {
	free(arr);
	arr = NULL;

    } else {
	register int i;

	for (i = 0; i < initial_size; i++)
	    arr->data[i] = NULL;

	arr->allocated = initial_size;
	arr->inctype = TN_ARRAY_INCGEOMETRICAL;
        arr->incstep = 2;

        arr->free_fn = freef;
        
        if (cmpf)
            arr->cmp_fn = cmpf;
        else
            arr->cmp_fn = trurl_default_cmpf;
    }

    return arr;
}
#endif


#ifdef MODULE_n_array_ctl_growth
tn_array *n_array_ctl_growth(tn_array *arr, int inctype, int incstep)
{
    if (incstep < 2 && inctype != TN_ARRAY_INCNONE) {
	trurl_die("n_array_new_ex: incstep less than 2(%d)\n", incstep);
	return NULL;
    }
    
    arr->inctype = inctype;
    arr->incstep = incstep;
    return arr;
}
#endif


#ifdef MODULE_n_array_clean
tn_array *n_array_clean(tn_array *arr)
{
    register unsigned int i;

    for (i = 0; i < arr->allocated; i++) {
        if (arr->data[i] != NULL) {
            if (arr->free_fn != NULL)
                arr->free_fn(arr->data[i]);
                
            arr->data[i] = NULL;
        }
    }
    
    arr->start_index = 0;
    arr->items = 0;

    return arr;
}
#endif


#ifdef MODULE_n_array_free
void n_array_free(tn_array *arr)
{
    n_array_clean(arr);

    free(arr->data);
    arr->data = NULL;
    free(arr);
}
#endif


#ifdef MODULE_n_array_grow_priv
static tn_array *n_array_realloc(tn_array *arr, size_t new_size)
{
    register int diff;

    diff = new_size - arr->allocated;

    n_assert(diff > 0);

    if (diff > 0) {
	void **tmp;

	if ((tmp = realloc(arr->data, new_size * sizeof(*tmp))) == NULL) {
	    return NULL;

	} else {
	    register size_t i;
	    for (i = arr->allocated; i < new_size; i++)
		tmp[i] = NULL;

	    arr->data = tmp;
	    arr->allocated = new_size;
	}
    }
    return arr;
}


tn_array *n_array_grow_priv_(tn_array *arr, size_t req_size)
{
    register size_t new_size = arr->allocated;

    while (req_size >= new_size) {
        switch (arr->inctype) {

            case TN_ARRAY_INCNONE:
                trurl_die("n_array_grow: grow request for const size array");
                return NULL;
                
            case TN_ARRAY_INCLINEAR:
                new_size += arr->incstep;
                break;

            case TN_ARRAY_INCGEOMETRICAL:
                new_size *= arr->incstep;
                break;
                
            default:
                n_assert(0);
                trurl_die("n_array_grow: unknown inctype %d", arr->inctype);
	}
    }
    
    return n_array_realloc(arr, new_size);
}
#endif

#if 0
int n_array_size(const tn_array *arr)
{
    return arr->items;
}
#endif


#ifdef MODULE_n_array_nth
void *n_array_nth(const tn_array *arr, int i)
{
    register unsigned int pos = arr->start_index + i;

    n_assert(i >= 0);
    n_assert(arr->allocated > 0);
    n_assert(arr->items > 0);

    if ((size_t) i >= arr->items || i < 0) {
	trurl_die("n_array_nth: index(%d) out of bounds(%d)\n", i, arr->items);
	return NULL;
    }
    
    return arr->data[pos];
}
#endif


#ifdef MODULE_n_array_remove_nth
tn_array *n_array_remove_nth(tn_array *arr, int i)
{
    register unsigned int pos = arr->start_index + i;
    register void *ptr;

    n_assert(i >= 0);
    n_assert(arr->allocated > 0);
    n_assert(arr->items > 0);

    if ((size_t) i >= arr->items || i < 0) {
	trurl_die("n_array_remove_nth: index(%d) out of bounds(0 - %d)\n", i,
		  arr->items);
	return NULL;
    }
    
    ptr = arr->data[pos];

    /* if slot is not empty, free node data */
    if (arr->data[pos] != NULL && arr->free_fn != NULL)
	arr->free_fn(arr->data[pos]);
    
    
    memmove(&arr->data[pos], &arr->data[pos + 1],
	    (arr->allocated - 1 - pos) * sizeof(*arr->data));

    arr->data[arr->allocated - 1] = NULL;
    arr->items--;

    return arr;
}
#endif


#ifdef MODULE_n_array_set_nth
tn_array *n_array_set_nth(tn_array *arr, int i, void *data)
{
    register size_t pos = arr->start_index + i;

    n_assert(i >= 0);
    n_assert(arr->allocated > 0);

    if (i < 0) {
	trurl_die("n_array_set_nth: index(%d) out of bounds(%d)\n", i,
                  arr->items);
	return NULL;
    }
    
    if (pos > arr->allocated) {
        if (n_array_grow_priv_(arr, pos) == NULL)
	    return NULL;

	arr->items = pos + 1;

    } else {
        /* if slot is not empty, free node */
	if (arr->data[pos] != NULL && arr->free_fn != NULL) {
	    arr->free_fn(arr->data[pos]);
	}
    }

    arr->data[pos] = data;
    return arr;
}
#endif


#ifdef MODULE_n_array_push
tn_array *n_array_push(tn_array *arr, void *data)
{
    if (arr->items == arr->allocated) {
        if (n_array_grow_priv_(arr, arr->allocated + 1) == NULL)
	    return NULL;
    }
    
    arr->data[arr->start_index + arr->items] = data;
    arr->items++;

    return arr;
}
#endif 


#ifdef MODULE_n_array_pop
void *n_array_pop(tn_array *arr)
{
    register void *ptr;
    register size_t pos = arr->start_index + arr->items - 1;

    if (arr->items <= 0) {
	trurl_die("tn_array_pop: pop from empty array\n");
	return NULL;
    }

    if (arr->items > 0)
	arr->items--;

    ptr = arr->data[pos];

    arr->data[pos] = NULL;

    return ptr;
}
#endif


#ifdef MODULE_n_array_shift
void *n_array_shift(tn_array *arr)
{
    void *ptr;

    if (arr->items <= 0) {
	trurl_die("tn_array_shift: shift from empty array\n");
	return NULL;
    }
    arr->items--;

    ptr = arr->data[arr->start_index];
    arr->data[arr->start_index] = NULL;
    arr->start_index++;

    return ptr;
}
#endif


#ifdef MODULE_n_array_unshift
tn_array *n_array_unshift(tn_array *arr, void *data)
{
    if(arr->start_index > 0) {
	arr->start_index--;
	arr->data[arr->start_index] = data;

    } else {
	if (arr->items == arr->allocated) {
	    if (n_array_grow_priv_(arr, arr->allocated + 1) == NULL)
		return NULL;
	}
        
	memmove(&arr->data[1], &arr->data[0], arr->items * sizeof(*arr->data));
	arr->data[0] = data;
    }

    arr->items++;
    return arr;
}
#endif


#ifdef MODULE_n_array_uniq_ex
tn_array *n_array_uniq_ex(tn_array *arr, t_fn_cmp cmpf)
{
    register size_t i = 1;

    if (cmpf == NULL)
	cmpf = arr->cmp_fn;

    n_assert(cmpf != NULL);

    if (cmpf == NULL) {
	trurl_die("n_array_uniq_ex: compare function is NULL\n");
	return NULL;
    }
    
    while (i < arr->items) {
	if (cmpf(arr->data[i - 1], arr->data[i]) == 0) {
	    n_array_remove_nth(arr, i);

	} else {
	    i++;
	}
    }

    return arr;
}
#endif


#ifdef MODULE_n_array_eq_ex
int n_array_eq_ex(const tn_array *arr1, const tn_array *arr2, t_fn_cmp cmpf)
{
    register int i, n = n_array_size(arr1);

    if (cmpf == NULL)
	cmpf = arr1->cmp_fn;

    n_assert(cmpf != NULL);

    if (cmpf == NULL) {
	trurl_die("n_array_eq_ex: compare function is NULL\n");
	return -1;
    }
    
    if (n_array_size(arr1) != n_array_size(arr2))
	return 0;

    for (i = 0; i < n; i++) {
	if (cmpf(arr1->data[i], arr2->data[i]) != 0)
	    return 0;
    }

    return 1;
}
#endif


#ifdef MODULE_n_array_dup
tn_array *n_array_dup(const tn_array *arr, t_fn_dup dup_fn)
{
    tn_array *dupl;

    n_assert(dup_fn != NULL);

    if (dup_fn == NULL)
	return NULL;

    dupl = n_array_new(arr->allocated, arr->free_fn, arr->cmp_fn);
    n_array_ctl_growth(dupl, arr->inctype, arr->incstep);

    if (dupl != NULL) {
	register unsigned int i;

	int n = arr->start_index;
	for (i = 0; i < arr->items; i++)
	    n_array_push(dupl, dup_fn(arr->data[n + i]));
    }

    n_assert(dupl->items == arr->items);

    return dupl;
}
#endif


#ifdef MODULE_n_array_sorts

#define SWAP_void(a, b)               \
            { register void *tmp = a; \
              a = b;                  \
              b = tmp;                \
            }



/* Function is based on: 

   **   
   ** quicksort.c -- quicksort integer array
   ** public domain by Raymond Gardner     12/91
 */
static
void qsort_voidp_arr(void **arr, size_t arr_size, t_fn_cmp cmpf)
{
    register unsigned i, j, ln, rn;

    while (arr_size > 1) {

	SWAP_void(arr[0], arr[arr_size / 2]);

	for (i = 0, j = arr_size;;) {

	    do {
		j--;
	    } while (cmpf(arr[j], arr[0]) > 0);


	    do {
		i++;
	    } while (i < j && cmpf(arr[i], arr[0]) < 0);

	    if (i >= j)
		break;

	    SWAP_void(arr[i], arr[j]);
	}

	SWAP_void(arr[j], arr[0]);

	ln = j;
	rn = arr_size - ++j;

	if (ln < rn) {
	    qsort_voidp_arr(arr, ln, cmpf);
	    arr += j;
	    arr_size = rn;

	} else {
	    qsort_voidp_arr(&arr[j], rn, cmpf);
	    arr_size = ln;
	}
    }
}


static
void isort_voidp_arr(void **arr, size_t arr_size, t_fn_cmp cmpf)
{
    register size_t i, j;

    for (i = 1; i < arr_size; i++) {
	register void *tmp = arr[i];

	j = i;

	while (j > 0 && cmpf(tmp, arr[j - 1]) < 0) {
	    arr[j] = arr[j - 1];
	    j--;
	}

	arr[j] = tmp;
    }
}


tn_array *n_array_sort_ex(tn_array *arr, t_fn_cmp cmpf)
{
    if (cmpf == NULL)
	cmpf = arr->cmp_fn;

    if (cmpf == NULL) {
	trurl_die("n_array_sort_ex: compare function is NULL\n");
	return NULL;
    }
    
#if 1
    /* there is error in qsort_viodp_arr  - PawelK*/
    /* where is it? - PawelG */
    if (arr->items > 10) {
	qsort_voidp_arr(&arr->data[arr->start_index], arr->items, cmpf);

    } else {
	isort_voidp_arr(&arr->data[arr->start_index], arr->items, cmpf);
    }
#else
	isort_voidp_arr(&arr->data[arr->start_index], arr->items, cmpf);
#endif

    return arr;
}


tn_array *n_array_qsort_ex(tn_array *arr, t_fn_cmp cmpf)
{
    if (cmpf == NULL)
	cmpf = arr->cmp_fn;

    if (cmpf == NULL) {
	trurl_die("n_array_sort_ex: compare function is NULL\n");
	return NULL;
    }
    
    qsort_voidp_arr(&arr->data[arr->start_index], arr->items, cmpf);
    return arr;
}


tn_array *n_array_isort_ex(tn_array *arr, t_fn_cmp cmpf)
{
    if (cmpf == NULL)
	cmpf = arr->cmp_fn;

    if (cmpf == NULL) {
	trurl_die("n_array_sort_ex: compare function is NULL\n");
	return NULL;
    }
    
    isort_voidp_arr(&arr->data[arr->start_index], arr->items, cmpf);
    return arr;
}
#endif


#ifdef MODULE_n_array_bsearch_ex
static __inline__
int bsearch_voidp_arr(void *const *arr, size_t arr_size, const void *data,
                      t_fn_cmp cmpf)
{
    register size_t l, r, i;
    int cmp_res;

    l = 0;
    r = arr_size;

    while (l < r) {

	i = (l + r) / 2;

	if ((cmp_res = cmpf(arr[i], data)) == 0) {
	    return i;
            
	} else if (cmp_res > 0) {
	    r = i;

	} else if (cmp_res < 0) {
	    l = i + 1;
	}
    }

    return -1;
}


int n_array_bsearch_idx_ex(const tn_array *arr, const void *data, t_fn_cmp cmpf)
{
    int idx;
    void **base;

    if (cmpf == NULL) {
        if (arr->cmp_fn == NULL) {
            trurl_die("n_array_bsearch_ex: compare function is NULL\n");
            return -1;
        }
        cmpf = arr->cmp_fn;
    }
    
    if(arr->items == 0)
        return -1;
    
    base = &arr->data[arr->start_index];

    if (arr->items > 1) {
	idx = bsearch_voidp_arr(base, arr->items, data, cmpf);
        if (idx > 0) {
            while (idx) {
                if (cmpf(base[idx - 1], data) != 0)
                    break;
                idx--;
            }
        }

    } else {
        idx = -1;
	if (cmpf(base[0], data) == 0)
	    idx = 0;
    }

    return idx;
}


void *n_array_bsearch_ex(const tn_array *arr, const void *data, t_fn_cmp cmpf)
{
    int idx;
    void **base;

    if (cmpf == NULL) {
        if (arr->cmp_fn == NULL) {
            trurl_die("n_array_bsearch_ex: compare function is NULL\n");
            return NULL;
        }
        cmpf = arr->cmp_fn;
    }
    
    if(arr->items == 0)
        return NULL;
    
    base = &arr->data[arr->start_index];

    if (arr->items > 1) {
	idx = bsearch_voidp_arr(base, arr->items, data, cmpf);
        
    } else {
        idx = -1;
	if (cmpf(base[0], data) == 0)
	    idx = 0;
    }

    return idx < 0 ? NULL : base[idx];
}
#endif

#ifdef MODULE_n_array_map
void n_array_map(tn_array *arr, void (*map_fn) (void *))
{
    register size_t i, n;

    n = arr->items;

    for (i = arr->start_index; i < n; i++)
	map_fn(arr->data[i]);

}
#endif


#ifdef MODULE_n_array_map_arg
void n_array_map_arg(tn_array *arr, void (*map_fn) (void *, void *), void *arg)
{
    register size_t i, n;

    n = arr->items;

    for (i = arr->start_index; i < n; i++)
	map_fn(arr->data[i], arg);

}
#endif


#ifndef NDEBUG
#ifdef MODULE_n_array_dump_stats
void n_array_dump_stats(const tn_array *arr, const char *name)
{

    printf("\nArray \"%s\" [memsize, items, start_index] = %d, %d, %d\n",
           name ? name : "", arr->allocated, arr->items, arr->start_index);
}
#endif
#endif
