/* $Id$ */

#include "n_array_int.h"


#define SWAP_void(a, b)               \
            { register void *tmp = a; \
              a = b;                  \
              b = tmp;                \
            }



/* Function is based on: 

   **   
   ** quicksort.c -- quicksort integer array
   ** public domain by Raymond Gardner     12/91
 */
static
void qsort_voidp_arr(void **arr, size_t arr_size, t_fn_cmp cmpf)
{
    register unsigned i, j, ln, rn;

    while (arr_size > 1) {

	SWAP_void(arr[0], arr[arr_size / 2]);

	for (i = 0, j = arr_size;;) {

	    do {
		j--;
	    } while (j && cmpf(arr[j], arr[0]) > 0);


	    do {
		i++;
	    } while (i < j && cmpf(arr[i], arr[0]) < 0);

	    if (i >= j)
		break;

	    SWAP_void(arr[i], arr[j]);
	}

	SWAP_void(arr[j], arr[0]);

	ln = j;
	rn = arr_size - ++j;

	if (ln < rn) {
	    qsort_voidp_arr(arr, ln, cmpf);
	    arr += j;
	    arr_size = rn;

	} else {
	    qsort_voidp_arr(&arr[j], rn, cmpf);
	    arr_size = ln;
	}
    }
}


static
void isort_voidp_arr(void **arr, size_t arr_size, t_fn_cmp cmpf)
{
    register size_t i, j;

    for (i = 1; i < arr_size; i++) {
	register void *tmp = arr[i];

	j = i;

	while (j > 0 && cmpf(tmp, arr[j - 1]) < 0) {
	    arr[j] = arr[j - 1];
	    j--;
	}

	arr[j] = tmp;
    }
}

static inline t_fn_cmp autosort(tn_array *arr, t_fn_cmp cmpf, int *set_sorted) 
{
    *set_sorted = 1;
    
    if (cmpf == NULL) {
        if (arr->cmp_fn == NULL) {
            trurl_die("n_array_sort: compare function is NULL\n");
            return NULL;
        }

        cmpf = arr->cmp_fn;
        
    } else if (cmpf != arr->cmp_fn) {
        SETARR_UNSORTED(arr);
        *set_sorted = 0;
    }

    return cmpf;
}

#define SORT_SORT  0
#define SORT_QSORT 1
#define SORT_ISORT 2

static inline tn_array *n_array_sort_internal(tn_array *arr, t_fn_cmp cmpf, int alg)
{
    int set_sorted;

    cmpf = autosort(arr, cmpf, &set_sorted);

    if ((arr->flags & TN_ARRAY_AUTOSORTED) && ISARR_SORTED(arr))
        return arr;
    
    switch (alg) {
        case SORT_SORT:
            if (arr->items > 10) 
                qsort_voidp_arr(&arr->data[arr->start_index], arr->items, cmpf);
            else 
                isort_voidp_arr(&arr->data[arr->start_index], arr->items, cmpf);
            break;

        case SORT_QSORT:
            qsort_voidp_arr(&arr->data[arr->start_index], arr->items, cmpf);
            break;
            
        case SORT_ISORT:
            isort_voidp_arr(&arr->data[arr->start_index], arr->items, cmpf);
            break;
    }
    
    
    if (set_sorted)    
        arr->flags |= ARR_SORTED;
    
    return arr;
}

tn_array *n_array_sort_ex(tn_array *arr, t_fn_cmp cmpf) 
{
    return n_array_sort_internal(arr, cmpf, SORT_SORT);
}

tn_array *n_array_qsort_ex(tn_array *arr, t_fn_cmp cmpf) 
{
    return n_array_sort_internal(arr, cmpf, SORT_QSORT);
}

tn_array *n_array_isort_ex(tn_array *arr, t_fn_cmp cmpf) 
{
    return n_array_sort_internal(arr, cmpf, SORT_ISORT);
}
