#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include "n_array_int.h"

#include <trurl/nmalloc.h>

tn_array *n_array_init_ex(tn_array *arr, int size, t_fn_free freef,
                          t_fn_cmp cmpf, void **data)
{
    n_assert(size >= 0);
    n_assert(arr);

    if (data) {
        arr->data   = data;
        arr->items  = size;

    } else {
#ifdef TRURL_VERY_PORTABLE
        register int i;
#endif
        if (size < 1)
            size = 2;

        arr->data = n_malloc(size * sizeof(*arr->data));
#ifdef TRURL_VERY_PORTABLE
        for (i = 0; i < size; i++)
            arr->data[i] = NULL;
#else
        memset(arr->data, 0, size * sizeof(*arr->data));
#endif
        arr->items     = 0;
    }

    arr->_refcnt   = 0;
    arr->flags     = 0;
    arr->allocated = size;
    arr->start_index  = 0;

    arr->free_fn = freef;
    arr->cmp_fn = trurl_default_cmpf;

    if (cmpf)
        arr->cmp_fn = cmpf;

    return arr;
}

tn_array *n_array_new_ex(int size, t_fn_free freef, t_fn_cmp cmpf, void **data)
{
    tn_array *arr;
#if 0
    static int n = 0;
    n++;
    printf("n_array %d %d\n", n, n * sizeof(*arr));
#endif
    arr = n_malloc(sizeof(*arr));
    return n_array_init_ex(arr, size, freef, cmpf, data);
}

#undef n_array_new              /* legacy */
tn_array *n_array_new(int size, t_fn_free freef, t_fn_cmp cmpf)
{
    return n_array_new_ex(size, freef, cmpf, NULL);
}


tn_array *n_array_new_na(tn_alloc *na, int size, t_fn_cmp cmpf)
{
    tn_array *arr;

    arr = na->na_malloc(na, sizeof(*arr));
    n_array_init_ex(arr, size, NULL, cmpf, NULL);
    arr->flags |= TN_ARRAY_INTERNAL_NA;
    return arr;
}

void n_array_free(tn_array *arr)
{
    if (arr->_refcnt > 0) {
        arr->_refcnt--;
        return;
    }
    n_assert((arr->flags & TN_ARRAY_INTERNAL_NA) == 0);

    n_array_unfreeze(arr);
    n_array_clean(arr);
    free(arr->data);
    arr->data = NULL;
    free(arr);
}

void n_array_cfree(tn_array **arrptr)
{
    if (*arrptr) {
        n_array_free(*arrptr);
        *arrptr = NULL;
    }
}

void n_array_free_na(tn_alloc *na, tn_array *arr)
{
    if (arr->_refcnt > 0) {
        arr->_refcnt--;
        return;
    }
    n_assert((arr->flags & TN_ARRAY_INTERNAL_NA) == 1);

    n_array_clean(arr);
    free(arr->data);
    arr->data = NULL;
    na->na_free(na, arr);
}

tn_fn_free n_array_ctl_set_freefn(tn_array *arr, tn_fn_free free_fn)
{
    tn_fn_free old_free_fn = arr->free_fn;
    arr->free_fn = free_fn;
    return old_free_fn;
}

tn_fn_cmp n_array_ctl_set_cmpfn(tn_array *arr, tn_fn_cmp cmp_fn)
{
    tn_fn_cmp old_cmp_fn = arr->cmp_fn;
    arr->cmp_fn = cmp_fn;
    TN_ARRAY_clr_sorted(arr);
    return old_cmp_fn;
}

tn_fn_cmp n_array_ctl_get_cmpfn(const tn_array *arr)
{
    return arr->cmp_fn;
}
