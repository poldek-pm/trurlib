/* 
  TRURLib
  obstack's wrapper
  $Id$
*/

#ifndef TRURL_OBSTACK_H
#define TRURL_OBSTACK_H

#include <stdint.h>
#include <obstack.h>

#include <trurl/ndie.h>
#include <trurl/n_obj_ref.h>
#include <trurl/nmalloc.h>

#define obstack_chunk_alloc n_malloc
#define obstack_chunk_free  n_free

#define TN_OBSTACK_REFCNT  (1 << 0)

/* WARN: never ever access struct members directly */
struct trurl_obstack_private {
    uint16_t        _refcnt;
    uint16_t        flags;
    struct obstack  ob;
    void* (*alloc)(struct trurl_obstack_private *ob, size_t size);
};

typedef struct trurl_obstack_private tn_obstack;

static inline
void *n_obstack_alloc(tn_obstack *ob, size_t size);

static inline
tn_obstack *n_obstack_new(int chunk_kb) 
{
    tn_obstack *nob = n_malloc(sizeof(*nob));
    nob->_refcnt = 0;
    obstack_init(&nob->ob);
    if (chunk_kb < 4) 
        chunk_kb = 4;
    obstack_chunk_size(&nob->ob) = 1024 * chunk_kb;
    nob->alloc = n_obstack_alloc;
    return nob;
}

static inline
void n_obstack_free(tn_obstack *ob, void *chunk) 
{
    if (chunk) {
        obstack_free(&ob->ob, chunk);
        return;
    }
    
    if (ob->_refcnt > 0) {
        ob->_refcnt--;
        return;
    }
    
    obstack_free(&ob->ob, NULL);
    n_free(ob);
}

static inline
void *n_obstack_alloc_ex(tn_obstack *ob, size_t size, int flags) 
{
    if (flags & TN_OBSTACK_REFCNT)
        ob->_refcnt++; 
    return obstack_alloc(&ob->ob, size);
}

static inline
void *n_obstack_alloc(tn_obstack *ob, size_t size) 
{
    return obstack_alloc(&ob->ob, size);
}

static inline
void *n_obstack_realloc(tn_obstack *ob, void *ptr, size_t deltasize) 
{
    obstack_grow(&ob->ob, ptr, deltasize);
    return ptr;
}


#endif /* TRURL_OBSTACK_H */
