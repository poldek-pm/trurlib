/* $Id$ */

#include "n_array_int.h"

void *n_array_nth(const tn_array *arr, int i)
{
    n_assert(i >= 0);
    n_assert(arr->allocated > 0);
    n_assert(arr->items > 0);

    if ((size_t) i >= arr->items || i < 0) {
	trurl_die("n_array_nth: index(%d) out of bounds(%d)\n", i, arr->items);
	return NULL;
    }
    
    return arr->data[arr->start_index + i];
}
