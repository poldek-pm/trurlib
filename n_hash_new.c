/* $Id$ */

#include "n_hash_int.h"


static unsigned int hash_string(const char *string);

tn_hash *n_hash_new_ex(size_t size, void (*freefn) (void *),
                       unsigned int (*hashfn) (const char*))
{
    tn_hash *ht = NULL;

    ht = calloc(1, sizeof(*ht));

    if (ht == NULL)
	return NULL;

    ht->table = calloc(size, sizeof(*ht->table));

    if (ht->table == NULL) {
	free(ht);
	return NULL;
    }
    ht->size = size;
    ht->items = 0;
    ht->free_fn = freefn;
    ht->hash_fn = hashfn ? hashfn : (unsigned (*)(const char*))hash_string;
    
    return ht;
}

int n_hash_size(const tn_hash *ht) 
{
    return ht->items;
}


#if USE_HASHSTRING_GLIBC_DB
# include "hash-string.h"
#elif USE_HASHSTRING_CDB
# define CDB_HASHSTART 5381
static unsigned int hash_string(const char *s)
{
  unsigned int v;

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
