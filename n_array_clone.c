/* $Id$ */

#include "n_array_int.h"

tn_array *n_array_clone_ex(const tn_array *arr, t_fn_dup dup_fn)
{
    tn_array *dupl;
    
    dupl = n_array_new(arr->allocated, arr->free_fn, arr->cmp_fn);
    dupl->flags = arr->flags;
    if (dup_fn != NULL) {
        register unsigned int i;
        int n = arr->start_index;
        for (i = 0; i < arr->items; i++)
            n_array_push(dupl, dup_fn(arr->data[n + i]));
        n_assert(dupl->items == arr->items);
    }

    return dupl;
}

/* legacy */
tn_array *n_array_clone(const tn_array *arr)
{
    return n_array_clone_ex(arr, NULL);
}

/* legacy */
tn_array *n_array_dup(const tn_array *arr, t_fn_dup dup_fn)
{
    return n_array_clone_ex(arr, dup_fn);
}

