/* 
  TRURLib
  $Id$
*/

#ifndef TRURL_OBJ_REF_H
#define TRURL_OBJ_REF_H

#include <stdint.h>
#include <trurl/nassert.h>

/* o is a trurlib object: its first member is a uint32_t reference count */
static inline void *n_ref(void *o)
{
    register uint32_t *v;

    v = (uint32_t*)o;
    n_assert(*v < UINT32_MAX);
    (*v)++;
    return o;
}

#endif
