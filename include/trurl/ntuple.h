/* 
  TRURLib
  Dynamic array of void*
  $Id$
*/

#ifndef TRURL_TUPLE_H
#define TRURL_TUPLE_H

#include <stdint.h>
#include <trurl/ndie.h>
#include <trurl/n_obj_ref.h>

typedef struct trurl_tuple_private {
    uint16_t    _refcnt;
    uint16_t    size;
    void        *data[0];
} tn_tuple;

tn_tuple *n_tuple_new(tn_alloc *na, int size, void **data);
void n_tuple_free(tn_alloc *na, tn_tuple *tu);

int n_tuple_size(const tn_tuple *tu);
#define n_tuple_size(tup) n_tuple_size_inl(tup)

#ifndef SWIG
static inline int n_tuple_size_inl(const tn_tuple *tu)
{
    return tu->size;
}
#endif

/*
  tup[i] = data ;
*/
void *n_tuple_set_nth(tn_tuple *tu, int i, void *data);
#define n_tuple_set_nth(a, i, d) n_tuple_set_nth_inl(a, i, d)

extern const char *n_errmsg_tuple_nth_oob;

#ifndef SWIG
static inline
tn_tuple *n_tuple_set_nth_inl(tn_tuple *tu, register int i, void *data)
{
    if ((size_t) i >= tu->size || i < 0)
        n_die(n_errmsg_tuple_nth_oob, i, tu->size);
    
    tu->data[i] = data;
    return tu;
}
#endif


/*
  foo = tup[i];
*/
void *n_tuple_nth(const tn_tuple *tu, int i);
#define n_tuple_nth(a, i) n_tuple_nth_inl(a, i)

extern const char *n_errmsg_tuple_nth_oob;

#ifndef SWIG
static inline void *n_tuple_nth_inl(const tn_tuple *tu, register int i)
{
    if ((size_t) i >= tu->size || i < 0)
        n_die(n_errmsg_tuple_nth_oob, i, tu->size);
    
    return tu->data[i];
}
#endif


/* insertion sort*/
tn_tuple *n_tuple_isort_ex(tn_tuple *tu, t_fn_cmp cmpf);
tn_tuple *n_tuple_qsort_ex(tn_tuple *tu, t_fn_cmp cmpf);
tn_tuple *n_tuple_sort_ex(tn_tuple *tu, t_fn_cmp cmpf);

/* cmpf is always called as cmpf(tup[i], data);
 * NOTE: function does *not* sort an tuple
 */
void *n_tuple_bsearch_ex(const tn_tuple *tu, const void *data, t_fn_cmp cmpf);
#define n_tuple_bsearch(tup, data) n_tuple_bsearch_ex(tup, data, NULL)


/* same as above, but returns position number, if there are more
   than one the same items, always returns first position;
   If item not found returns -1; 
 */
int n_tuple_bsearch_idx_ex(const tn_tuple *tu, const void *data, t_fn_cmp cmpf);
#define n_tuple_bsearch_idx(tup, data) n_tuple_bsearch_idx_ex(tup, data, NULL)

#endif /* TRURL_TUPLE_H */
