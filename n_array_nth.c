#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include "n_array_int.h"

const char *n_errmsg_array_nth_oob = TN_ERRMSG_ARRAY_NTH_OOB;

#undef n_array_nth
void *n_array_nth(const tn_array *arr, int i)
{
    n_assert(i >= 0);
    n_assert(arr->allocated > 0);
    n_assert(arr->items > 0);

    if ((size_t) i >= arr->items || i < 0) {
        trurl_die(TN_ERRMSG_ARRAY_NTH_OOB, i, arr->items);
        return NULL;
    }
    
    return arr->data[arr->start_index + i];
}

#undef n_array_size
int n_array_size(const tn_array *arr)
{
    return arr->items;
}
