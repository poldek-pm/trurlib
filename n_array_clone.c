/* $Id$ */

#include "n_array_int.h"

tn_array *n_array_clone(const tn_array *arr)
{
    register unsigned i;
    tn_array *new_arr;

    if ((new_arr = calloc(1, sizeof(*new_arr))) == NULL)
	return NULL;
    
    *new_arr = *arr;
    new_arr->data = malloc(arr->allocated * sizeof(*arr->data));
    if (new_arr->data == NULL) {
	free(new_arr);
	arr = NULL;
    }
    
    new_arr->items = 0;
    new_arr->start_index = 0;
    
    for (i = 0; i < new_arr->allocated; i++)
        new_arr->data[i] = NULL;
    
    return new_arr;
}
