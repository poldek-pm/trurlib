#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include "n_array_int.h"

tn_array *n_array_remove_nth(tn_array *arr, int i)
{
    register unsigned int pos = arr->start_index + i;

    trurl_die__if_frozen(arr);
    
    n_assert(i >= 0);
    n_assert(arr->allocated > 0);
    n_assert(arr->items > 0);

    if ((size_t) i >= arr->items || i < 0) {
        trurl_die("n_array_remove_nth: index(%d) out of bounds(0 - %d)\n", i,
                  arr->items);
        return NULL;
    }
    
    /* if slot is not empty, free node data */
    if (arr->data[pos] != NULL && arr->free_fn != NULL)
        arr->free_fn(arr->data[pos]);

    if (pos == arr->items) 
        arr->data[pos] = NULL;
    else 
        memmove(&arr->data[pos], &arr->data[pos + 1],
                (arr->allocated - 1 - pos) * sizeof(*arr->data));

    arr->data[arr->allocated - 1] = NULL;
    arr->items--;
    return arr;
}
