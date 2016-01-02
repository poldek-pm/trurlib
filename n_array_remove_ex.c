#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include "n_array_int.h"

tn_array *n_array_remove_ex(tn_array *arr, const void *data, t_fn_cmp cmpf)
{
    register int i, items, n;
    register void *ptr;

    trurl_die__if_frozen(arr);
    
    if (arr->items == 0)
        return arr;
    
    if (cmpf == NULL)
        cmpf = arr->cmp_fn;

    n_assert(cmpf != NULL);
    
    i = arr->start_index;
    items = arr->items;

    while ((size_t)i < arr->allocated) {
        ptr = arr->data[i];
        if (ptr == NULL || cmpf(ptr, data) != 0) {
            i++;
            continue;
        }
        
        /* if slot is not empty, free node data */
        if (arr->free_fn != NULL)
            arr->free_fn(ptr);
        
        n = arr->allocated - 1 - i;
        n_assert(n >= 0);
        
        if (n > 0)
            memmove(&arr->data[i], &arr->data[i + 1], n * sizeof(*arr->data));
        
        arr->data[arr->allocated - 1] = NULL;
        items--;
    }
    arr->items = items;
    return arr;
}
