/* 
  TRURLib
  Dynamic array of void*
  $Id$
*/

#ifndef TRURL_ARRAY_H
#define TRURL_ARRAY_H

#include <stdint.h>
#include <trurl/tfn_types.h>
#include <trurl/ndie.h>
#include <trurl/n_obj_ref.h>
#include <trurl/nmalloc.h>

#define TN_ARRAY_CONSTSIZE         (1 << 0)
#define TN_ARRAY_AUTOSORTED        (1 << 1) /* an array sorts itself in bsearch_*
                                               functions;  don't work with
                                               external cmp functions */
#define TN_ARRAY_INTERNAL_ISSORTED  (1 << 8)
#define TN_ARRAY_INTERNAL_NA        (1 << 9)
/* WARN: _never_ ever access array members directly */
typedef struct trurl_array_private {
    uint16_t    _refcnt;
    uint16_t    flags;
    
    size_t      items;
    size_t      allocated;
    size_t      start_index;

    void        **data;
    
    t_fn_free   free_fn;
    t_fn_cmp    cmp_fn;

} tn_array;


//typedef struct trurl_array_private tn_array;

tn_array *n_array_new_ex(int size, t_fn_free freef, t_fn_cmp cmpf, void **data);
#define n_array_new(size, freef, cmpf) n_array_new_ex(size, freef, cmpf, NULL)
tn_array *n_array_new_na(int size, tn_alloc *na, t_fn_cmp cmpf);

tn_array *n_array_init_ex(tn_array *arr, int size,
                          t_fn_free freef, t_fn_cmp cmpf, void **data);

#define n_array_init(arr, size, freef, cmpf) \
            n_array_init_ex(arr, size, freef, cmpf, NULL)


#ifndef SWIG
static inline tn_array *n_array_ctl(tn_array *arr, unsigned flags) {
    arr->flags |= flags;
    return arr;
}
#endif
#define n_array_ctl_growth(arr, inctype)  ((void) 0) /* backward API compat */
#define n_array_has_free_fn(arr) (arr)->free_fn

tn_fn_free n_array_ctl_set_freefn(tn_array *arr, tn_fn_free free_fn);
tn_fn_cmp n_array_ctl_set_cmpfn(tn_array *arr, tn_fn_cmp cmp_fn);


void n_array_free(tn_array *arr);
void n_array_free_na(tn_alloc *na, tn_array *arr);

/*
  Free content 
 */
tn_array *n_array_clean(tn_array *arr);

/*
  Clone an array.
 */
tn_array *n_array_clone_ex(const tn_array *arr, t_fn_dup dupf);
/* aliases for n_array_clone_ex */
tn_array *n_array_clone(const tn_array *arr); 
tn_array *n_array_dup(const tn_array *arr, t_fn_dup dupf);

/* 
   for(i=0; i < n_array_size(arr); i++) 
        ...
*/

int n_array_size(const tn_array *arr);
#define n_array_size(arr) n_array_size_inl(arr)

#ifndef SWIG
static inline int n_array_size_inl(const tn_array *arr)
{
    return arr->items;
}
#endif
#define n_array_isempty(arr) (n_array_size(arr) == 0)


/*
  foo = arr[i];
*/
void *n_array_nth(const tn_array *arr, int i);
#define n_array_nth(a, i) n_array_nth_inl(a, i)

extern const char *n_errmsg_array_nth_oob;

#ifndef SWIG
static inline void *n_array_nth_inl(const tn_array *arr, register int i)
{
    if ((size_t) i >= arr->items || i < 0)
        n_die(n_errmsg_array_nth_oob, i, arr->items);
    
    return arr->data[arr->start_index + i];
}
#endif

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
#define n_array_push(a, d) n_array_push_inl(a, d)

tn_array *n_array_concat_ex(tn_array *arr, tn_array *src, tn_fn_dup dup_fn);
#define n_array_concat(arr, src) n_array_concat_ex(arr, src, NULL) 

/* internal macros don't use */

#define TN_ARRAY_set_sorted(arr) ((arr)->flags |= TN_ARRAY_INTERNAL_ISSORTED)
#define TN_ARRAY_clr_sorted(arr) ((arr)->flags &= ~TN_ARRAY_INTERNAL_ISSORTED)
#define TN_ARRAY_is_sorted(arr)  ((arr)->flags &  TN_ARRAY_INTERNAL_ISSORTED)

tn_array *n_array_grow_priv_(tn_array *arr, size_t req_size);
#ifndef SWIG
static inline tn_array *n_array_push_inl(tn_array *arr, void *data) {

    if (arr->items == arr->allocated) {
        if (n_array_grow_priv_(arr, arr->allocated + 1) == 0)
            return 0;
    }
    
    arr->data[arr->start_index + arr->items] = data;
    arr->items++;
    TN_ARRAY_clr_sorted(arr);
    return arr;
}
#endif
#if 0
#ifndef TN_ARRAY_INTERNAL
# undef TN_ARRAY_INTERNAL_ISSORTED
# undef TN_ARRAY_set_sorted
# undef TN_ARRAY_clr_sorted
# undef TN_ARRAY_is_sorted
#endif
#endif

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
#define n_array_isort(arr) n_array_isort_ex(arr, NULL)


/* cmpf is always called as cmpf(arr[i], data);
 * NOTE: function does *not* sort an array
 */
void *n_array_bsearch_ex(const tn_array *arr, const void *data, t_fn_cmp cmpf);
#define n_array_bsearch(arr, data) n_array_bsearch_ex(arr, data, NULL)


/* same as above, but returns position number, if there are more
   than one the same items, always returns first position;
   If item not found returns -1; 
 */
int n_array_bsearch_idx_ex(const tn_array *arr, const void *data, t_fn_cmp cmpf);
#define n_array_bsearch_idx(arr, data) n_array_bsearch_idx_ex(arr, data, NULL)




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
