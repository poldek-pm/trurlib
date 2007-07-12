/* $Id$ */

#include "n_hash_int.h"

static tn_hash *do_n_hash_new_ex(tn_alloc *na, 
                                 size_t size, void (*freefn) (void *))
{
    tn_hash *ht = NULL;

    if (na == NULL)
        ht = n_calloc(1, sizeof(*ht));
    else
        ht = na->na_calloc(na, sizeof(*ht));
        ht->na = na;

    if (ht == NULL)
        return NULL;

    ht->flags = 0;
    ht->table = n_calloc(size, sizeof(*ht->table));

#if ENABLE_TRACE    
    if (size % 2 == 0 && size > 100)
        printf("n_hash_new %p %d\n", ht, size);
#endif    
    
    ht->size = size;
    ht->free_fn = freefn;
    ht->hash_fn = NULL;
    ht->_refcnt = 0;
    ht->na = na;
    return ht;
}

static
tn_hash *n_hash_new2(tn_alloc *na, size_t size, void (*freefn) (void *)) 
{
    register size_t rsize = 4;

    while (rsize < size)
        rsize <<= 1;

    return do_n_hash_new_ex(na, rsize, freefn);
}


tn_hash *n_hash_new_ex(size_t size, void (*freefn) (void *),
                       unsigned int (*hashfn) (const char*))
{
    tn_hash *ht = NULL;

    if (hashfn == NULL)
        return n_hash_new2(NULL, size, freefn);
    
    ht = do_n_hash_new_ex(NULL, size, freefn);
    ht->hash_fn = hashfn;
    return ht;
}

tn_hash *n_hash_new_na(tn_alloc *na, size_t size, void (*freefn) (void *))
{
    if (na == NULL) 
        na = n_alloc_new(16, TN_ALLOC_OBSTACK);
    else
        na = n_ref(na);
    
    return n_hash_new2(na, size, freefn);
}

/* djb hash */
# define CDB_HASHSTART 5381
int n_hash_dohash(const tn_hash *ht, const char *s, int *slen)
{
    register unsigned int v;
    const char *ss = s;
    
    if (ht->hash_fn)
        return ht->hash_fn(s) % ht->size;

    v = CDB_HASHSTART;
    while (*s) {
        v += (v << 5);
        v ^= (unsigned)*s;
        s++;
    }

    if (slen)
        *slen = s - ss;

    return v & (ht->size - 1);
}

#if 0                           /* disabled */
/*
  FNV hash (see http://www.isthe.com/chongo/tech/comp/fnv)
  A bit slower and gives a bit lower collision rate than djb's
*/
int n_hash_dohash(const tn_hash *ht, const char *s, int *slen)
{
    register unsigned v = 0;
    const char *ss = s;
    
    while (*s) {
        /* xor the bottom with the current octet */
        v ^= (unsigned)*s++;
        // v += (v<<1) + (v<<4) + (v<<7) + (v<<8) + (v<<24);
        v *= 0x01000193;
    }
    
    if (slen)
        *slen = s - ss;

    return v & (ht->size - 1);
}
#endif

int n_hash_size(const tn_hash *ht) 
{
    return ht->items;
}

