/* $Id$ */

#include "n_array_int.h"

void n_array_dump_stats(const tn_array *arr, const char *name)
{

    printf("\nArray \"%s\" [memsize, items, start_index] = %zd, %zd, %zd\n",
           name ? name : "", arr->allocated, arr->items, arr->start_index);
}
