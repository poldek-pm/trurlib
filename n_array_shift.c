/* $Id$ */

#include "n_array_int.h"

void *n_array_shift(tn_array *arr)
{
    void *ptr;

    if (arr->items <= 0) {
	trurl_die("tn_array_shift: shift from empty array\n");
	return NULL;
    }
    arr->items--;

    ptr = arr->data[arr->start_index];
    arr->data[arr->start_index] = NULL;
    arr->start_index++;
    return ptr;
}
