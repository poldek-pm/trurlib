/* $Id$ */

#include "n_hash_int.h"


static unsigned int hash_string(const char *string);

static tn_hash *do_n_hash_new_ex(size_t size, void (*freefn) (void *))
{
    tn_hash *ht = NULL;

    ht = n_calloc(1, sizeof(*ht));

    if (ht == NULL)
        return NULL;

    ht->flags = 0;
    ht->table = n_calloc(size, sizeof(*ht->table));
    if (ht->table == NULL) {
        free(ht);
        return NULL;
    }

#if ENABLE_TRACE    
    if (size % 2 == 0 && size > 100)
        printf("n_hash_new %p %d\n", ht, size);
#endif    
        
    ht->size = size;
    ht->items = 0;
    ht->free_fn = freefn;
    ht->hash_fn = NULL;
    ht->_refcnt = 0;
    return ht;
}

static
tn_hash *n_hash_new2(size_t size, void (*freefn) (void *)) 
{
    register size_t rsize = 4;

    while (rsize < size)
        rsize <<= 1;

    return do_n_hash_new_ex(rsize, freefn);
}


tn_hash *n_hash_new_ex(size_t size, void (*freefn) (void *),
                       unsigned int (*hashfn) (const char*))
{
    tn_hash *ht = NULL;

    if (hashfn == NULL)
        return n_hash_new2(size, freefn);
    
    ht = do_n_hash_new_ex(size, freefn);
    ht->hash_fn = hashfn;
    return ht;
}


# define CDB_HASHSTART 5381
int n_hash_dohash(const tn_hash *ht, const char *s, int *slen)
{
    register unsigned int v, n;

    if (ht->hash_fn)
        return ht->hash_fn(s) % ht->size;
        
    
    v = CDB_HASHSTART;
    n = 0;
    while (*s) {
        v += (v << 5);
        v ^= *s;
        s++;
        n++;
    }
    if (slen)
        *slen = n;
    //v += n;
    return v & (ht->size - 1);
}



int n_hash_size(const tn_hash *ht) 
{
    return ht->items;
}

#if USE_HASHSTRING_GLIBC_DB
# include "hash-string.h"
#elif USE_HASHSTRING_CDB
static unsigned int hash_string(const char *s)
{
  register unsigned int v;

  v = CDB_HASHSTART;
  while (*s) {
      v += (v << 5);
      v ^= *s;
      s++;
  }
  return v;
}

#else
/*
** Hashes a string to produce an unsigned int, which should be
** sufficient for most purposes.
*/
static unsigned int hash_string(const char *string)
{
    unsigned ret_val = 0;
    int i;

    while (*string) {
        i = (long int) (*(string++));
        ret_val ^= i;
        ret_val <<= 1;
    }
    return ret_val;
}
#endif
