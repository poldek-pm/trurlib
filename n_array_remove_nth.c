/* $Id$ */

#include "n_array_int.h"

tn_array *n_array_remove_nth(tn_array *arr, int i)
{
    register unsigned int pos = arr->start_index + i;
    register void *ptr;

    n_assert(i >= 0);
    n_assert(arr->allocated > 0);
    n_assert(arr->items > 0);

    if ((size_t) i >= arr->items || i < 0) {
	trurl_die("n_array_remove_nth: index(%d) out of bounds(0 - %d)\n", i,
		  arr->items);
	return NULL;
    }
    
    ptr = arr->data[pos];

    /* if slot is not empty, free node data */
    if (arr->data[pos] != NULL && arr->free_fn != NULL)
	arr->free_fn(arr->data[pos]);
    
    
    memmove(&arr->data[pos], &arr->data[pos + 1],
	    (arr->allocated - 1 - pos) * sizeof(*arr->data));

    arr->data[arr->allocated - 1] = NULL;
    arr->items--;
    SETARR_UNSORTED(arr);
    return arr;
}
