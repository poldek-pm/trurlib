/*
  TRURLib

  $Id$
 */

#ifndef TRURL_HASH_H
#define TRURL_HASH_H


#include <stddef.h>           /* For size_t     */
#include <trurl/n_obj_ref.h>
#include <trurl/nmalloc.h>

typedef struct trurl_hash_table tn_hash;

/*
** This is used to construct the table.  If it doesn't succeed,
** returns NULL
*/

tn_hash *n_hash_new_ex(size_t size, void (*freefn) (void *),
                       unsigned int (*hashfn) (const char*));

#define n_hash_new(a, b) n_hash_new_ex(a, b, NULL)

tn_hash *n_hash_new_na(tn_alloc *na, size_t size, void (*freefn) (void *));

void n_hash_free(tn_hash *ht);

int n_hash_size(const tn_hash *ht);

#define TN_HASH_NOCPKEY    (1 << 1)  /* do not make own copies of the keys */
#define TN_HASH_REHASH     (1 << 2)  /* automatically grow and rehash
                                        if filled more than 80%  */
#define TN_HASH_NOREPLACE  (1 << 3)  /* don't check for duplicate keys
                                        while inserting; use with care! */

int n_hash_ctl(tn_hash *ht, unsigned int flags);

/* Removes all entries */
void n_hash_clean(tn_hash *ht);


/*
  Inserts a pointer to 'data' in the table, with a copy of 'key'
  (if TN_HASH_NOCPKEY not set) as its key.  Note that this makes
  a copy of the key, but NOT of the associated data.
*/

tn_hash *n_hash_insert(tn_hash *ht, const char *key, const void *data);

tn_hash *n_hash_replace(tn_hash *ht, const char *key, const void *data);


/*
  Returns a pointer to the data associated with a key.
*/
void *n_hash_get(const tn_hash *ht, const char *key);


int n_hash_exists(const tn_hash *ht, const char *key);


/*
  lower level insert / exists pair to avoid double computation of key hash
 */

int n_hash_exists_ex(const tn_hash *ht,
                     const char *key, int *klen, unsigned *khash);

void *n_hash_get_ex(const tn_hash *ht,
                    const char *key, int *klen, unsigned *khash);

tn_hash *n_hash_insert_ex(tn_hash *ht,
                          const char *key, int klen, unsigned khash,
                          const void *data);


/* low level api */
uint32_t n_hash_compute_hash(const tn_hash *ht,
                             const char *key, int klen);

int n_hash_hexists(const tn_hash *ht,
                   const char *key, int klen, uint32_t khash);

void *n_hash_hget(const tn_hash *ht,
                  const char *key, int klen, uint32_t khash);

tn_hash *n_hash_hinsert(tn_hash *ht,
                        const char *key, int klen, unsigned khash,
                        const void *data);

tn_hash *n_hash_hreplace(tn_hash *ht,
                         const char *key, int klen, unsigned khash,
                         const void *data);

uint32_t n_hash_compute_raw_hash(const char *key, int klen);
uint32_t n_hash_compute_index_hash(const tn_hash *ht, uint32_t raw_hash);



/* iterator */
struct trurl_hash_iterator {
    tn_hash *ht;
    int pos;
    int bpos;
};

typedef struct trurl_hash_iterator tn_hash_it;
void n_hash_it_init(tn_hash_it *hi, tn_hash *ht);
void *n_hash_it_get(tn_hash_it *hi, const char **key);

/*
** Deletes an entry from the table.  Returns a pointer to the data that
** was associated with the key so the calling code can dispose of it
** properly.
*/
void *n_hash_remove(tn_hash *ht, const char *key);


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

tn_array *n_hash_values(const tn_hash *ht);

#include <trurl/tfn_types.h>
tn_hash *n_hash_dup(const tn_hash *ht, tn_fn_dup dup_fn);

int n_hash_stats(const tn_hash *ht);
#endif /* TRURL_HASH_H */
