#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include "n_array_int.h"

tn_array *n_array_clean(tn_array *arr)
{
    register unsigned int i;
    
    trurl_die__if_frozen(arr);

    for (i = 0; i < arr->allocated; i++) {
        if (arr->data[i] != NULL) {
            if (arr->free_fn != NULL)
                arr->free_fn(arr->data[i]);
                
            arr->data[i] = NULL;
        }
    }
    
    arr->start_index = 0;
    arr->items = 0;

    return arr;
}
