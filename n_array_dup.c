/* $Id$ */

#include "n_array_int.h"

tn_array *n_array_dup(const tn_array *arr, t_fn_dup dup_fn)
{
    tn_array *dupl;

    n_assert(dup_fn != NULL);

    if (dup_fn == NULL)
	return NULL;

    dupl = n_array_new(arr->allocated, arr->free_fn, arr->cmp_fn);
    dupl->flags = arr->flags;
    if (dupl != NULL) {
	register unsigned int i;

	int n = arr->start_index;
	for (i = 0; i < arr->items; i++)
	    n_array_push(dupl, dup_fn(arr->data[n + i]));
    }

    n_assert(dupl->items == arr->items);

    return dupl;
}
