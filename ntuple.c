#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "trurl_internal.h"
#include "nmalloc.h"
#include "tfn_types.h"
#include "ntuple.h"

#include <trurl/nmalloc.h>

const char *n_errmsg_tuple_nth_oob = "n_tuple_nth: index(%d) out of bounds(%d)\n";

tn_tuple *n_tuple_new(tn_alloc *na, int size, void **data)
{
    tn_tuple *tu;

    n_assert(size < UINT16_MAX);
    if (na)
        tu = na->na_malloc(na, sizeof(*tu) + (size * sizeof(void*)));
    else
        tu = n_malloc(sizeof(*tu) + (size * sizeof(void*)));
    
    tu->_refcnt = 0;
    tu->size = size;
    if (data)
        memcpy(tu->data, data, size * sizeof(void*));
    else if (size)
        memset(tu->data, 0, size * sizeof(void*));
    return tu;
}
#include <trurl/narray.h>
tn_tuple *n_tuple_new_fromarray(tn_alloc *na, tn_array *arr)
{
    tn_tuple *tu = n_tuple_new(na, n_array_size(arr), NULL);
    register int i;

    i = 0;
    while (n_array_size(arr) > 0)
        tu->data[i++] = n_array_shift(arr);
    return tu;
}


void n_tuple_free(tn_alloc *na, tn_tuple *tu)
{
    if (tu->_refcnt > 0) {
        tu->_refcnt--;
        return;
    }
    tu->size = 0;
    if (na)
        na->na_free(na, tu);
    else
        n_free(tu);
}

#undef n_tuple_size
int n_tuple_size(const tn_tuple *tu)
{
    return tu->size;
}

#undef n_tuple_nth
void *n_tuple_nth(const tn_tuple *tu, register int i)
{
    
    if ((size_t) i >= tu->size || i < 0)
        n_die(n_errmsg_tuple_nth_oob, i, tu->size);
    
    return tu->data[i];
}

extern
void trurl_isort_voidp_arr(void **arr, size_t arr_size, t_fn_cmp cmpf);

extern
void trurl_qsort_voidp_arr(void **arr, size_t arr_size, t_fn_cmp cmpf);


#define SORT_SORT  0
#define SORT_QSORT 1
#define SORT_ISORT 2


static inline tn_tuple *n_tuple_sort_internal(tn_tuple *tu, t_fn_cmp cmpf, int alg)
{
    switch (alg) {
        
        case SORT_SORT:
            if (tu->size > 10) 
                trurl_qsort_voidp_arr(&tu->data[0], tu->size, cmpf);
            else 
                trurl_isort_voidp_arr(&tu->data[0], tu->size, cmpf);
            break;

        case SORT_QSORT:
            trurl_qsort_voidp_arr(&tu->data[0], tu->size, cmpf);
            break;
            
        case SORT_ISORT:
            trurl_isort_voidp_arr(&tu->data[0], tu->size, cmpf);
            break;
    }
    
    return tu;
}

tn_tuple *n_tuple_sort_ex(tn_tuple *tu, t_fn_cmp cmpf) 
{
    return n_tuple_sort_internal(tu, cmpf, SORT_SORT);
}

tn_tuple *n_tuple_qsort_ex(tn_tuple *tu, t_fn_cmp cmpf) 
{
    return n_tuple_sort_internal(tu, cmpf, SORT_QSORT);
}

tn_tuple *n_tuple_isort_ex(tn_tuple *tu, t_fn_cmp cmpf) 
{
    return n_tuple_sort_internal(tu, cmpf, SORT_ISORT);
}

static __inline__
int bsearch_voidp_tu(void *const *arr, size_t arr_size, const void *data,
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


int n_tuple_bsearch_idx_ex(const tn_tuple *tu, const void *data,
                           t_fn_cmp cmpf)
{
    int idx;
    void **base;
    
    if (tu->size == 0)
        return -1;
    
    base = (void**)tu->data;

    if (tu->size > 1) {
        idx = bsearch_voidp_tu(base, tu->size, data, cmpf);
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

void *n_tuple_bsearch_ex(const tn_tuple *tu, const void *data, t_fn_cmp cmpf)
{
    int idx;
    void **base;
    
    if (tu->size == 0)
        return NULL;
    
    base = (void**)tu->data;

    if (tu->size > 1) {
        idx = bsearch_voidp_tu(base, tu->size, data, cmpf);
        
    } else {
        idx = -1;
        
        if (cmpf(base[0], data) == 0)
            idx = 0;
    }

    return idx < 0 ? NULL : base[idx];
}










