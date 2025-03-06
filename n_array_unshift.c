#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include "n_array_int.h"

tn_array *n_array_unshift(tn_array *arr, void *data)
{
    trurl_die__if_frozen(arr);

    if(arr->start_index > 0) {
        arr->start_index--;
        arr->data[arr->start_index] = data;

    } else {
        if (arr->items == arr->allocated) {
            if (n_array_grow(arr, arr->allocated + 1) == NULL)
                return NULL;
        }

        memmove(&arr->data[1], &arr->data[0], arr->items * sizeof(*arr->data));
        arr->data[0] = data;
    }

    arr->items++;
    TN_ARRAY_clr_sorted(arr);
    return arr;
}
