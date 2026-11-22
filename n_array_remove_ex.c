#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include "n_array_int.h"

tn_array *n_array_remove_ex(tn_array *arr, const void *data, t_fn_cmp cmpf)
{
    size_t i, j, end;
    void *ptr;

    trurl_die__if_frozen(arr);

    if (arr->items == 0)
        return arr;

    if (cmpf == NULL)
        cmpf = arr->cmp_fn;

    n_assert(cmpf != NULL);

    /* one pass: each item kept is moved once, next to the previous one kept;
       a memmove() of the rest for each item removed made it O(n * removed) */
    end = arr->start_index + arr->items;
    for (i = j = arr->start_index; i < end; i++) {
        ptr = arr->data[i];
        if (ptr != NULL && cmpf(ptr, data) == 0) {
            /* if slot is not empty, free node data */
            if (arr->free_fn != NULL)
                arr->free_fn(ptr);
            continue;
        }
        arr->data[j++] = ptr;
    }

    arr->items = j - arr->start_index;
    while (j < end)
        arr->data[j++] = NULL;

    return arr;
}
