#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include "n_array_int.h"

static tn_array *n_array_realloc(tn_array *arr, size_t new_size)
{
    register int diff;
    void **tmp;

    diff = new_size - arr->allocated;
    n_assert(diff > 0);

    tmp = n_realloc(arr->data, new_size * sizeof(*tmp));
    memset(&tmp[arr->allocated], 0, (new_size - arr->allocated) * sizeof(*tmp));

    arr->data = tmp;
    arr->allocated = new_size;
    return arr;
}

tn_array *n_array_grow(tn_array *arr, size_t req_size)
{
    register size_t new_size = arr->allocated;

    while (req_size >= new_size) {
        if (arr->flags & TN_ARRAY_CONSTSIZE) {
            trurl_die("n_array_grow: grow request for const size array");
            return NULL;
        }

        new_size *= 2;
    }

    return new_size > arr->allocated ? n_array_realloc(arr, new_size) : arr;
}
