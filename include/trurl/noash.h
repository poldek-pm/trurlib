/*
  TRURLib
  Open addressing hash table
 */

#ifndef TRURL_OHASH_H
#define TRURL_OHASH_H

#include <stddef.h>           /* For size_t     */
#include <trurl/n_obj_ref.h>
#include <trurl/nmalloc.h>
#include <trurl/nstream.h>


typedef struct trurl_oash_table tn_oash;

/*
** This is used to construct the table.  If it doesn't succeed,
** returns NULL
*/
tn_oash *n_oash_new_ex(size_t size, void (*freefn) (void *),
                         unsigned int (*hashfn) (const char*));

#define n_oash_new(a, b) n_oash_new_ex(a, b, NULL)

tn_oash *n_oash_new_na(tn_alloc *na, size_t size, void (*freefn) (void *));

void n_oash_free(tn_oash *ht);

int n_oash_size(const tn_oash *ht);
int n_oash_ctl(tn_oash *ht, unsigned int flags);

/* Removes all entries */
void n_oash_clean(tn_oash *ht);

/*
  Inserts a pointer to 'data' in the table, with a copy of 'key'
  (if TN_OASH_NOCPKEY not set) as its key.  Note that this makes
  a copy of the key, but NOT of the associated data.
*/
tn_oash *n_oash_insert(tn_oash *ht, const char *key, const void *data);
tn_oash *n_oash_replace(tn_oash *ht, const char *key, const void *data);

/*
  Returns a pointer to the data associated with a key.
*/
void *n_oash_get(const tn_oash *ht, const char *key);

int n_oash_exists(const tn_oash *ht, const char *key);


/*
  lower level insert / exists pair to avoid double computation of key hash
 */
int n_oash_exists_ex(const tn_oash *ht,
                     const char *key, int klen, unsigned *khash);

void *n_oash_get_ex(const tn_oash *ht,
                    const char *key, int *klen, unsigned *khash);

tn_oash *n_oash_insert_ex(tn_oash *ht,
                          const char *key, int klen, unsigned khash,
                          const void *data);

/* low level api */
uint32_t n_oash_compute_hash(const tn_oash *ht,
                             const char *key, int klen);

int n_oash_hexists(const tn_oash *ht,
                   const char *key, int klen, uint32_t khash);

void *n_oash_hget(const tn_oash *ht,
                  const char *key, int klen, uint32_t khash);

tn_oash *n_oash_hinsert(tn_oash *ht,
                        const char *key, int klen, unsigned khash,
                        const void *data);

tn_oash *n_oash_hreplace(tn_oash *ht,
                         const char *key, int klen, unsigned khash,
                         const void *data);

uint32_t n_oash_compute_raw_hash(const char *key, int klen);
uint32_t n_oash_compute_index_hash(const tn_oash *ht, uint32_t raw_hash);

/* iterator */
struct trurl_ohash_iterator {
    tn_oash *ht;
    int pos;
};

typedef struct trurl_ohash_iterator tn_oash_it;
void n_oash_it_init(tn_oash_it *hi, tn_oash *ht);
void *n_oash_it_get(tn_oash_it *hi, const char **key);

/*
** Deletes an entry from the table.  Returns a pointer to the data that
** was associated with the key so the calling code can dispose of it
** properly.
*/
void *n_oash_remove(tn_oash *ht, const char *key);

/*
** Goes through a hash table and calls the function passed to it
** for each node that has been inserted.  The function is passed
** a pointer to the key, and a pointer to the data associated
** with it.
*/
int n_oash_map(const tn_oash *ht, void (*map_fn)(const char *, void *));

int n_oash_map_arg(const tn_oash *ht,
                   void (*map_fn)(const char *, void *, void *), void *arg);

#include <trurl/narray.h>
tn_array *n_oash_keys_ext(const tn_oash *ht, int cp);

#define n_oash_keys(ht) n_oash_keys_ext(ht, 0)
#define n_oash_keys_cp(ht) n_oash_keys_ext(ht, 1)

tn_array *n_oash_values(const tn_oash *ht);

#include <trurl/tfn_types.h>
tn_oash *n_oash_dup(const tn_oash *ht, tn_fn_dup dup_fn);


int n_oash_store(const tn_oash *ht, tn_stream *st,
                 int (*store_fn) (void *, tn_stream *, void*), void *store_ctx);

tn_oash *n_oash_restore(tn_stream *st,
                        void *(*restore_fn) (tn_stream *, void*), void *restore_ctx,
                        void (*freefn) (void *));

int n_oash_stats(const tn_oash *ht);

#endif /* TRURL_OHASH_H */
