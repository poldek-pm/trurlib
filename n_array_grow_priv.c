#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include "n_array_int.h"

static tn_array *n_array_realloc(tn_array *arr, size_t new_size)
{
    void **tmp;

    n_assert(new_size > arr->allocated);

    tmp = n_realloc(arr->data, new_size * sizeof(*tmp));
    memset(&tmp[arr->allocated], 0, (new_size - arr->allocated) * sizeof(*tmp));

    arr->data = tmp;
    arr->allocated = new_size;
    return arr;
}

/* room for req_size items; the slots n_array_shift() left at the front are
   used first */
tn_array *n_array_grow(tn_array *arr, size_t req_size)
{
    register size_t new_size = arr->allocated;

    n_array__adjust(arr);

    while (req_size > new_size) {
        if (arr->flags & TN_ARRAY_CONSTSIZE) {
            trurl_die("n_array_grow: grow request for const size array");
            return NULL;
        }

        /* arr->allocated is a uint32_t; on ILP32 size_t would wrap too */
        if (new_size > UINT32_MAX / 2) {
            trurl_die("n_array_grow: %zu items do not fit in an array", req_size);
            return NULL;
        }

        new_size *= 2;
    }

    return new_size > arr->allocated ? n_array_realloc(arr, new_size) : arr;
}

void n_array__adjust(tn_array *arr) {
    if (arr->start_index == 0)
        return;

    if (arr->items > 0) {
        //DBGF("adjust i%d, a%d, s%d\n", arr->start_index, arr->allocated, arr->items);
        memmove(&arr->data[0], &arr->data[arr->start_index],
                arr->items * sizeof(*arr->data));
        memset(&arr->data[arr->items], 0, arr->start_index * sizeof(*arr->data));
    }

    arr->start_index = 0;
}
