/*
  TRURLib
  Functions typedefs
  $Id$
*/
#ifndef __TRURL_FN_TYPES_H
#define __TRURL_FN_TYPES_H

typedef void  (*t_fn_free) (void*);  
typedef void* (*t_fn_dup)  (void*);
typedef int   (*t_fn_cmp)  (const void *, const void *);

#endif
