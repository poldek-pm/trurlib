/* $Id$ */

#include "n_array_int.h"

tn_array *n_array_remove_ex(tn_array *arr, const void *data, t_fn_cmp cmpf)
{
    register unsigned int i, items;
    register void *ptr;

    if (arr->items == 0)
        return arr;
    
    if (cmpf == NULL)
	cmpf = arr->cmp_fn;

    n_assert(cmpf != NULL);
    
    i = arr->start_index;
    items = arr->start_index + arr->items;
    items--;
    
    while (i < items) {
        if (cmpf(arr->data[i], data) != 0)
            i++;
        else {
            ptr = arr->data[i];
            
            /* if slot is not empty, free node data */
            if (arr->data[i] != NULL && arr->free_fn != NULL)
                arr->free_fn(arr->data[i]);
            
            memmove(&arr->data[i], &arr->data[i + 1],
                    (arr->allocated - 1 - i) * sizeof(*arr->data));
            items--;
        }
    }

    items++;
    
    if (cmpf(arr->data[items - 1], data) == 0) {
        items--;

        ptr = arr->data[i];
            
        /* if slot is not empty, free node data */
        if (arr->data[i] != NULL && arr->free_fn != NULL)
            arr->free_fn(arr->data[i]);
    }
    
    arr->items = items;
    return arr;
}
