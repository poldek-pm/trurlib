#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include "n_hash_int.h"

static
tn_hash *n_hash_put_node(tn_hash *ht, struct hash_bucket *node, int replace);

static void n_hash_rehash(tn_hash *ht)
{
    size_t i, newsize, oldsize;
    struct hash_bucket **oldtable;

    oldsize  = ht->size;
    oldtable = ht->table;

    DBGF("n_hash_rehash %d\n", ht->size);

    newsize = ht->size << 1;
    ht->table = n_realloc(ht->table, newsize * sizeof(*ht->table));
    if (ht->table == NULL) {
        ht->table = oldtable;
        return;
    }

    memset(&ht->table[ht->size], 0, (newsize - oldsize) * sizeof(*ht->table));
    ht->size = newsize;
    ht->items = 0;

    for (i = 0; i < oldsize; i++) {
        if (oldtable[i] == NULL)
            continue;

        n_hash_put_node(ht, oldtable[i], 0);
    }

    free(oldtable);
}

int n_hash_bucket_psldiff(const tn_hash *ht, const struct hash_bucket *e, uint32_t index)
{
    uint32_t psl;

    n_assert(e);

    uint32_t index0 = n_hash_index(ht->size, e->hash);

    if (index0 <= index) {
        psl = index - index0;
    } else {
        psl = index + (ht->size - index0);
    }

    return psl;
}

static
tn_hash *n_hash_put_node(tn_hash *ht, struct hash_bucket *node, int replace)
{
    struct hash_bucket **table = ht->table;
    struct hash_bucket *e;
    n_assert(node->klen > 0);

    node->psl = 0;
    n_assert(node->hash != 0);

    uint32_t index = node->hash & (ht->size - 1);

    e = table[index];
    if (e == NULL) {
        ht->items++;
        table[index] = node;
        return ht;
    }

    while (1) {
        node->psl++;

        index = n_hash_index(ht->size, index + 1);
        e = table[index];

        if (e == NULL) {
            ht->items++;
            table[index] = node;
            return ht;
        }

        if (n_hash_bucket_samekey(e, node)) {
            if (!replace) {
                trurl_die("key '%s' already in table\n", e->key);
                return NULL;
            }

            if (e->data) {
                if (ht->free_fn == NULL)
                    trurl_die("key '%s' replace memleak\n", e->key);
                else
                    ht->free_fn(e->data);
            }

            e->data = (void *)node->data;

            if (!ht->na) /* inefficient */
                free(e);

            return ht;
        }

        if (node->psl > e->psl) { /* swap */
            table[index] = node;
            node = e;
        }
    }

    return ht;
}

struct hash_bucket *n_hash_new_bucket(tn_hash *ht, const char *key, int klen,
                                      uint32_t khash)
{
    struct hash_bucket *e;
    int buflen = 0;

    n_assert(klen > 0);

    if ((ht->flags & TN_HASH_NOCPKEY) == 0)
        buflen = klen > 0 ? klen + 1 : (int)strlen(key) + 1;

    if (ht->na)
        e = ht->na->na_malloc(ht->na, sizeof(*e) + buflen);
    else
        e = n_malloc(sizeof(*e) + buflen);

    e->klen = klen;

    if (buflen == 0) {
        e->key = (char*)key;
    } else {
        memcpy(e->_buf, key, buflen);
        e->key = e->_buf;
    }

    e->psl = 0;
    e->hash = khash;
    e->data = NULL;
    return e;
}

static
tn_hash *n_hash_put(tn_hash *ht, const char *key, int klen, uint32_t khash,
                    const void *data, int replace)
{
    struct hash_bucket *e;

    n_assert(klen > 0);
    n_assert(klen < UINT16_MAX);

    if ((ht->flags & TN_HASH_REHASH) && ht->size - ht->items <= ht->size / 3) {
        n_hash_rehash(ht);
    }

    if (ht->size == ht->items) {
        trurl_die("n_hash: table is full");
    }

    if (khash == 0)
        khash = n_hash_compute_raw_hash(key, klen);

    e = n_hash_new_bucket(ht, key, klen, khash);
    e->data = (void *) data;

    return n_hash_put_node(ht, e, replace);
}

tn_hash *n_hash_insert(tn_hash *ht, const char *key, const void *data)
{
    return n_hash_put(ht, key, strlen(key), 0, data, 0);
}

tn_hash *n_hash_hinsert(tn_hash *ht, const char *key, int klen, unsigned khash,
                        const void *data)
{
    return n_hash_put(ht, key, klen, khash, data, 0);
}

tn_hash *n_hash_hreplace(tn_hash *ht, const char *key, int klen, unsigned khash,
                         const void *data)
{
    if (ht->flags & TN_HASH_NOREPLACE) {
        trurl_die("n_hash_replace: replace requested for"
                  " \"non-replace\" hash table");
        return NULL;
    }

    return n_hash_put(ht, key, klen, khash, data, 1);
}

tn_hash *n_hash_replace(tn_hash *ht, const char *key, const void *data)
{
    if (ht->flags & TN_HASH_NOREPLACE) {
        trurl_die("n_hash_replace: replace requested for"
                  " \"non-replace\" hash table");
        return NULL;
    }

    return n_hash_hreplace(ht, key, strlen(key), 0, data);
}

/* backward compat */
tn_hash *n_hash_insert_ex(tn_hash *ht, const char *key, int klen, unsigned khash,
                          const void *data) {
    return n_hash_hinsert(ht, key, klen, khash, data);
}
