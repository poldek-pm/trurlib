/*
  TRURLib
  Functions typedefs
  $Id$
*/
#ifndef TRURL_FN_TYPES_H
#define TRURL_FN_TYPES_H

typedef void  (*t_fn_free) (void*);  
typedef void* (*t_fn_dup)  (void*);
typedef int   (*t_fn_cmp)  (const void *, const void *);

typedef void  (*tn_fn_free) (void*);  
typedef void* (*tn_fn_dup)  (void*);
typedef int   (*tn_fn_cmp)  (const void *, const void *);

#endif
