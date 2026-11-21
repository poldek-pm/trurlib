#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include "n_array_int.h"

void n_array_dump_stats(const tn_array *arr, const char *name)
{

    printf("\nArray \"%s\" [memsize, items, start_index] = %u, %u, %u\n",
           name ? name : "", (unsigned)arr->allocated, (unsigned)arr->items,
           (unsigned)arr->start_index);
}
