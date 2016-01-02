#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include "n_array_int.h"
#undef n_array_ctl
#if 0
tn_array *n_array_ctl(tn_array *arr, unsigned flags)
{
    arr->flags |= flags;
    return arr;
}
#endif
