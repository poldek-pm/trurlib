/*
  TRURLib
  
  $Id$
 */

#ifndef TRURL_HASH_H
#define TRURL_HASH_H

#include <stddef.h>           /* For size_t     */

typedef struct trurl_hash_table tn_hash;

/*
** This is used to construct the table.  If it doesn't succeed,
** returns NULL
*/

tn_hash *n_hash_new_ex(size_t size, void (*freefn) (void *),
                       unsigned int (*hashfn) (const char*));

#define n_hash_new(a, b) n_hash_new_ex(a, b, NULL)


void n_hash_free(tn_hash *ht);

int n_hash_size(const tn_hash *ht);

#define TN_HASH_NOCPKEY    (1 << 1)  /* do not make own copies of the keys */
#define TN_HASH_REHASH     (1 << 2)  /* automatically grow and rehash
                                        if filled more than 80%  */
int n_hash_ctl(tn_hash *ht, unsigned int flags);
//int n_hash_ctl_bktallocfn(tn_hash *ht, );


/* Removes all entries */
void n_hash_clean(tn_hash *ht);


/*
** Inserts a pointer to 'data' in the table, with a copy of 'key' 
** (if TN_HASH_NOCPKEY not set) as its key.  Note that this makes 
** a copy of the key, but NOT of the associated data.
*/

tn_hash *n_hash_insert(tn_hash *ht, const char *key, const void *data);
tn_hash *n_hash_replace(tn_hash *ht, const char *key, const void *data);

/*
** Returns a pointer to the data associated with a key.
*/

void *n_hash_get(const tn_hash *ht, const char *key);


/*
** If the key has not been inserted in the table, returns 0,
** otherwise returns 1.
*/

int n_hash_exists(const tn_hash *ht, const char *key);

/*
** Deletes an entry from the table.  Returns a pointer to the data that
** was associated with the key so the calling code can dispose of it
** properly.
*/

void *n_hash_remove(tn_hash *table, const char *key);


/*
** Goes through a hash table and calls the function passed to it
** for each node that has been inserted.  The function is passed
** a pointer to the key, and a pointer to the data associated
** with it.
*/

int n_hash_map(const tn_hash *ht, void (*map_fn)(const char *, void *));

int n_hash_map_arg(const tn_hash *ht,
                   void (*map_fn)(const char *, void *, void *), void *arg);

#include <trurl/narray.h>
tn_array *n_hash_keys_ext(const tn_hash *ht, int cp);

#define n_hash_keys(ht) n_hash_keys_ext(ht, 0)
#define n_hash_keys_cp(ht) n_hash_keys_ext(ht, 1)

#include <trurl/tfn_types.h>
tn_hash *n_hash_dup(const tn_hash *ht, tn_fn_dup dup_fn);

int n_hash_stats(const tn_hash *ht);
#endif /* TRURL_HASH_H */
