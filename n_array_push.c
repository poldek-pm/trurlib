/* $Id$ */

#include "n_array_int.h"
#undef n_array_push

tn_array *n_array_push(tn_array *arr, void *data)
{
    trurl_die__if_frozen(arr);
    
    if (arr->items == arr->allocated) {
        if (n_array_grow_priv_(arr, arr->allocated + 1) == NULL)
            return NULL;
    }
    
    arr->data[arr->start_index + arr->items] = data;
    arr->items++;
    TN_ARRAY_clr_sorted(arr);
    return arr;
}


tn_array *n_array_concat_ex(tn_array *arr, tn_array *src, tn_fn_dup dup_fn)
{
    register int i;

    trurl_die__if_frozen(arr);
    
    for (i=0; i < n_array_size(src); i++) {
        void *item = n_array_nth(src, i);
        if (dup_fn)
            item = dup_fn(item);
        n_array_push(arr, item);
    }
    
    TN_ARRAY_clr_sorted(arr);
    return arr;
}
