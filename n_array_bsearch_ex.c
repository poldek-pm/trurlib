/* $Id$ */

#include "n_array_int.h"

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


int n_array_bsearch_idx_ex(const tn_array *arr, const void *data,
                           t_fn_cmp cmpf)
{
    int idx;
    void **base;
    
    if (cmpf == NULL) {
        if (arr->cmp_fn == NULL) {
            trurl_die("n_array_bsearch_ex: compare function is NULL\n");
            return -1;
        }

        if (arr->flags & TN_ARRAY_AUTOSORTED) 
            n_array_sort_ex((tn_array*)arr, NULL);
        
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
        if (arr->flags & TN_ARRAY_AUTOSORTED) 
            n_array_sort_ex((tn_array*)arr, NULL);
        
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
