#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include "n_array_int.h"

tn_array *n_array_uniq_ex(tn_array *arr, t_fn_cmp cmpf)
{
    size_t i, j, end;

    trurl_die__if_frozen(arr);

    if (cmpf == NULL)
        cmpf = arr->cmp_fn;

    n_assert(cmpf != NULL);

    if (cmpf == NULL) {
        trurl_die("n_array_uniq_ex: compare function is NULL\n");
        return NULL;
    }

    if (arr->items < 2)
        return arr;

    /* one pass: an item equal to the last one kept is freed, the others are
       moved once, next to it; n_array_remove_nth() of each duplicate made it
       O(n * duplicates) */
    end = arr->start_index + arr->items;
    for (i = j = arr->start_index + 1; i < end; i++) {
        void *ptr = arr->data[i];

        if (cmpf(arr->data[j - 1], ptr) == 0) {
            /* if slot is not empty, free node data */
            if (ptr != NULL && arr->free_fn != NULL)
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
