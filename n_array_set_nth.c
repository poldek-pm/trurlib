/* $Id$ */

#include "n_array_int.h"

tn_array *n_array_set_nth(tn_array *arr, int i, void *data)
{
    register size_t pos = arr->start_index + i;

    n_assert(i >= 0);
    n_assert(arr->allocated > 0);

    trurl_die__if_frozen(arr);
    
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
    TN_ARRAY_clr_sorted(arr);
    return arr;
}
