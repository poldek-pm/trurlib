/* $Id$ */

#include "n_array_int.h"

void n_array_free(tn_array *arr)
{
    n_array_clean(arr);

    free(arr->data);
    arr->data = NULL;
    free(arr);
}
