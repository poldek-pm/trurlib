/*
  TRURLib
  
  $Id$
 */
#ifndef TRURL_DBHASH_H
#define TRURL_DBHASH_H

#include <stddef.h>         /* For size_t     */


typedef struct trurl_dbhash tn_dbhash;

tn_dbhash *n_dbhash_new_ex(const char *file, int flags, int mode, void *hinfo);

#define n_dbhash_new(file, flags) n_dbhash_new_ex(file, flags, 0644, NULL)

void n_dbhash_free(tn_dbhash *ht);


struct ndbhash_data {
    void   *data;
    size_t size;
};

const struct ndbhash_data *n_dbhash_get(const tn_dbhash *ht, const char *key);
const void *n_dbhash_get_data(const tn_dbhash *ht, const char *key);



tn_dbhash *n_dbhash_insert(tn_dbhash *ht, const char *key,
                           const void *data, size_t data_size);

tn_dbhash *n_dbhash_replace(tn_dbhash *ht, const char *key,
                            const void *data, size_t data_size);


int n_dbhash_remove(tn_dbhash *ht, const char *key);


int n_dbhash_map(const tn_dbhash *ht,
                 void (*map_fn) (const void *, const void *, size_t));

int n_dbhash_map_arg(tn_dbhash *ht,
                     void (*map_fn) (const char*, const void *, size_t, void *),
                     void *arg);



#endif /* TRURL_DBHASH_H */
