#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include "n_array_int.h"

tn_array *n_array_uniq_ex(tn_array *arr, t_fn_cmp cmpf)
{
    register size_t i = 1;

    trurl_die__if_frozen(arr);
    
    if (cmpf == NULL)
        cmpf = arr->cmp_fn;

    n_assert(cmpf != NULL);

    if (cmpf == NULL) {
        trurl_die("n_array_uniq_ex: compare function is NULL\n");
        return NULL;
    }
    
    while (i < arr->items) {
        if (cmpf(arr->data[i - 1], arr->data[i]) == 0) {
            n_array_remove_nth(arr, i);

        } else {
            i++;
        }
    }
    return arr;
}
