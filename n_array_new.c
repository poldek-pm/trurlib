/* $Id$ */

#include "n_array_int.h"

tn_array *n_array_new_ex(int size, t_fn_free freef, t_fn_cmp cmpf, void **data)
{
    tn_array *arr;
    register int i;
    
    arr = n_malloc(sizeof(*arr));
    n_assert(size >= 0);
    
    if (data) {
        arr->data   = data;
        arr->items  = size;
        
    } else {
        if (size < 1)
            size = 2;
        
        arr->data = n_malloc(size * sizeof(*arr->data));
#ifdef TRURL_VERY_PORTABLE    
        for (i = 0; i < size; i++)
            arr->data[i] = NULL;
#else
        memset(arr->data, 0, size * sizeof(*arr->data));
#endif        
        arr->items     = 0;
    }

    arr->_refcnt   = 0;
    arr->flags     = 0;
    arr->allocated = size;
    arr->start_index  = 0;
    
    arr->free_fn = freef;
    arr->cmp_fn = trurl_default_cmpf;

    if (cmpf)
        arr->cmp_fn = cmpf;
    
    return arr;
}

#undef n_array_new
tn_array *n_array_new(int size, t_fn_free freef, t_fn_cmp cmpf)
{
    return n_array_new_ex(size, freef, cmpf, NULL);
}

