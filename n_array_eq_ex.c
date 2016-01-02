#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include "n_array_int.h"

int n_array_eq_ex(const tn_array *arr1, const tn_array *arr2, t_fn_cmp cmpf)
{
    register int i, n = n_array_size(arr1);

    if (cmpf == NULL)
	cmpf = arr1->cmp_fn;

    n_assert(cmpf != NULL);

    if (cmpf == NULL) {
	trurl_die("n_array_eq_ex: compare function is NULL\n");
	return -1;
    }
    
    if (n_array_size(arr1) != n_array_size(arr2))
	return 0;

    for (i = 0; i < n; i++) {
	if (cmpf(arr1->data[i], arr2->data[i]) != 0)
	    return 0;
    }

    return 1;
}
