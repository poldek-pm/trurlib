#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include "n_array_int.h"
#undef n_array_push

tn_array *n_array_push(tn_array *arr, void *data)
{
    trurl_die__if_frozen(arr);

    if (arr->items == arr->allocated) {
        if (n_array_grow(arr, arr->allocated + 1) == NULL)
            return NULL;
    }

    arr->data[arr->start_index + arr->items] = data;
    arr->items++;
    TN_ARRAY_clr_sorted(arr);
    return arr;
}


tn_array *n_array_concat_ex(tn_array *arr, const tn_array *src, tn_fn_dup dup_fn)
{
    register int i;

    trurl_die__if_frozen(arr);

    if (arr->items + src->items > arr->allocated) {
        if (n_array_grow(arr, arr->allocated + src->items) == NULL)
            return NULL;
    }

    register size_t index = arr->start_index + arr->items;
    for (i=0; i < n_array_size(src); i++) {
        void *item = n_array_nth(src, i);
        if (dup_fn)
            item = dup_fn(item);

        arr->data[index++] = item;
    }
    arr->items = index;
    TN_ARRAY_clr_sorted(arr);

    return arr;
}
