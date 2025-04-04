#ifndef TRURL_NARRAY_INT_H
#define TRURL_NARRAY_INT_H

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "trurl_internal.h"
#include "nmalloc.h"
#include "tfn_types.h"

#define TN_ARRAY_INTERNAL 1
#include "narray.h"

void n_array__adjust(tn_array *arr);

#endif /* TRURL_NARRAY_INT_H */
