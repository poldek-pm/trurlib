#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include "n_array_int.h"

void *n_array_shift(tn_array *arr)
{
    void *ptr;

    trurl_die__if_frozen(arr);
    
    if (arr->items <= 0) {
        trurl_die("tn_array_shift: shift from empty array\n");
        return NULL;
    }
    arr->items--;

    ptr = arr->data[arr->start_index];
    arr->data[arr->start_index] = NULL;
    arr->start_index++;
    if (arr->start_index == UINT16_MAX) /* start_index has 16 bits */
        n_array__adjust(arr);
    return ptr;
}
