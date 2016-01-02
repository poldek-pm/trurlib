#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include "n_array_int.h"

static tn_array *n_array_realloc(tn_array *arr, size_t new_size)
{
    register int diff;
    register size_t i;
    void **tmp;
    
    diff = new_size - arr->allocated;
    n_assert(diff > 0);

        
    tmp = n_realloc(arr->data, new_size * sizeof(*tmp));
        
    for (i = arr->allocated; i < new_size; i++)
        tmp[i] = NULL;

    arr->data = tmp;
    arr->allocated = new_size;
    return arr;
}


tn_array *n_array_grow_priv_(tn_array *arr, size_t req_size)
{
    register size_t new_size = arr->allocated;

    while (req_size >= new_size) {
        if (arr->flags & TN_ARRAY_CONSTSIZE) {
            trurl_die("n_array_grow: grow request for const size array");
            return NULL;
        }

        new_size *= 2;
    }
    
    return n_array_realloc(arr, new_size);
}
