/* $Id$ */

#include "n_array_int.h"

tn_array *n_array_new(int initial_size, t_fn_free freef, t_fn_cmp cmpf)
{
    tn_array *arr;

    if ((arr = calloc(1, sizeof(*arr))) == NULL)
	return NULL;

    n_assert(initial_size >= 0);
    if (initial_size < 1)
	initial_size = 2;

    if ((arr->data = malloc(initial_size * sizeof(*arr->data))) == NULL) {
	free(arr);
	arr = NULL;

    } else {
	register int i;

	for (i = 0; i < initial_size; i++)
	    arr->data[i] = NULL;

	arr->allocated = initial_size;
	arr->flags = 0;
        arr->free_fn = freef;
        
        if (cmpf)
            arr->cmp_fn = cmpf;
        else
            arr->cmp_fn = trurl_default_cmpf;
    }
    return arr;
}
