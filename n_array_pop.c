#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include "n_array_int.h"

void *n_array_pop(tn_array *arr)
{
    register void *ptr;
    register size_t pos = arr->start_index + arr->items - 1;

    trurl_die__if_frozen(arr);
    
    if (arr->items <= 0) {
        trurl_die("tn_array_pop: pop from empty array\n");
        return NULL;
    }

    if (arr->items > 0)
        arr->items--;

    ptr = arr->data[pos];

    arr->data[pos] = NULL;
    return ptr;
}
