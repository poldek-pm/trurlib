/* $Id$ */

#include "n_array_int.h"
#undef n_array_push

tn_array *n_array_push(tn_array *arr, void *data)
{
    //printf("n_array_push\n");
    if (arr->items == arr->allocated) {
        if (n_array_grow_priv_(arr, arr->allocated + 1) == NULL)
            return NULL;
    }
    
    arr->data[arr->start_index + arr->items] = data;
    arr->items++;
    TN_ARRAY_clr_sorted(arr);
    return arr;
}
