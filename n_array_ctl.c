/* $Id$ */

#include "n_array_int.h"

tn_array *n_array_ctl(tn_array *arr, unsigned flags)
{
    arr->flags |= flags;
    return arr;
}
