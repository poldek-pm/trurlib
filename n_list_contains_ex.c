/* $Id$ */

#include "n_list_int.h"

int n_list_contains_ex(const tn_list *l, const void *data, t_fn_cmp cmpf)
{
    return (n_list_lookup_ex(l, data, cmpf) != NULL);
}
