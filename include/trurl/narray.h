/* 
  TRURLib
  Dynamic array of void*
  $Id$
*/

#ifndef TRURL_ARRAY_H
#define TRURL_ARRAY_H

#include "tfn_types.h"
#include "ndie.h"

#define TN_ARRAY_CONSTSIZE         (1 << 0)

/* if changed array sorts itself in bsearch_* functions
   don't work with external cmp functions
 */
#define TN_ARRAY_AUTOSORTED        (1 << 1)

/* WARN: never ever access array members directly */
struct trurl_array_private {
    size_t      items;
    void        **data;
    size_t      allocated;
    unsigned    flags;
    size_t      start_index;

    t_fn_free   free_fn;
    t_fn_cmp    cmp_fn;
};

typedef struct trurl_array_private tn_array;

tn_array *n_array_new(int initial_size, t_fn_free freef, t_fn_cmp cmpf);
tn_array *n_array_ctl(tn_array *arr, unsigned flags);

#define n_array_ctl_growth(arr, inctype)  ((void) 0) /* backward API compat */

void n_array_free(tn_array *arr);

/*
  Free content 
 */
tn_array *n_array_clean(tn_array *arr);


/* Clone an array (only structure is cloned, not content) */
tn_array *n_array_clone(const tn_array *arr);

/* 
   for(i=0; i<n_array_size(arr); i++) 
       ...
*/
//#define n_array_size(arr) (*(int*)arr)
static inline int n_array_size(const tn_array *arr)
{
    return arr->items;
}

#define n_array_isempty(arr) (n_array_size(arr) == 0)

/*
  foo = arr[i];

*/
//void *n_array_nth(const tn_array *arr, int i);
static inline void *n_array_nth(const tn_array *arr, register int i)
{
    if ((size_t) i >= arr->items || i < 0)
	n_die("n_array_nth: index(%d) out of bounds(%d)\n", i, arr->items);
    
    return arr->data[arr->start_index + i];
}

/*
  NOTE:
  - grows array if 'i' is out of bounds.
  - if arr[i] exists destroy it by arr->free_fn(if set). Be careful!
  
  arr[i] = foo;

*/
tn_array *n_array_set_nth(tn_array *arr, int i, void *data);


/*
  memmove(&arr[i], &arr[i..LAST_INDEX])
  
*/
tn_array *n_array_remove_nth(tn_array *arr, int i);


/*
  arr[ LAST_INDEX++ ] = foo;

*/
tn_array *n_array_push(tn_array *arr, void *data);


/*
  foo = arr[ LAST_INDEX-- ];

*/
void *n_array_pop(tn_array *arr);


/*
  foo = arr[0];
  memmove(&arr[0], &arr[1..LAST_INDEX]);

*/
void *n_array_shift(tn_array *arr);


/*
  memmove(&arr[1], &arr[1..LAST_INDEX]);
  arr[0] = foo;

*/
tn_array *n_array_unshift(tn_array *arr, void *data);


/*
  return arr1 == arr2
  
 */
int n_array_eq_ex(const tn_array *arr1, const tn_array *arr2, t_fn_cmp cmpf);
#define n_array_eq(arr1, arr2) n_array_eq_ex(arr1, arr2, NULL)


/*
  Remove all duplicates
  NOTE: array must be sorted
*/
tn_array *n_array_uniq_ex(tn_array *arr, t_fn_cmp cmpf);
#define n_array_uniq(arr) n_array_uniq_ex(arr, NULL)

/* let function make decision about algorithm */
tn_array *n_array_sort_ex(tn_array *arr, t_fn_cmp cmpf);
#define n_array_sort(arr) n_array_sort_ex(arr, NULL)

/* quicksort */
tn_array *n_array_qsort_ex(tn_array *arr, t_fn_cmp cmpf);
#define n_array_qsort(arr) n_array_qsort_ex(arr, NULL)

/* insertion sort*/
tn_array *n_array_isort_ex(tn_array *arr, t_fn_cmp cmpf);
#define n_array_isort(arr) n_array_qsort_ex(arr, NULL)


/* cmpf is always called as cmpf(arr[i], data);
 * NOTE: function does *not* sort array
 */
void *n_array_bsearch_ex(const tn_array *arr, const void *data, t_fn_cmp cmpf);
#define n_array_bsearch(arr, data) n_array_bsearch_ex(arr, data, NULL)


/* same as above, but returns position number, if there are more
   than one the same items, always returns first position;
   If item not found returns -1; 
 */
int n_array_bsearch_idx_ex(const tn_array *arr, const void *data, t_fn_cmp cmpf);
#define n_array_bsearch_idx(arr, data) n_array_bsearch_idx_ex(arr, data, NULL)

/*
  Clone an array.
 */
tn_array *n_array_dup(const tn_array *arr, t_fn_dup dupf);


tn_array *n_array_remove_ex(tn_array *arr, const void *data, t_fn_cmp cmpf);
#define n_array_remove(arr, data) n_array_remove_ex(arr, data, NULL)

/* 
   for(i=0; i<n_array_size(arr); i++) 
       map_fn(arr[i])
*/
void n_array_map(tn_array *arr, void (*map_fn)(void *));


/* 
   for(i=0; i<n_array_size(arr); i++) 
       map_fn(arr[i], arg)
*/
void n_array_map_arg(tn_array *arr, void (*map_fn)(void *, void *), void *arg);


/* for debugging */
void n_array_dump_stats(const tn_array *arr, const char *name);


#endif /* TRURL_ARRAY_H */
