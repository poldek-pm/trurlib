#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include "n_hash_int.h"

static
tn_hash *n_hash_put_node(tn_hash *ht, struct hash_bucket *node);

static void n_hash_rehash(tn_hash *ht)
{
    size_t i, newsize, oldsize;
    struct hash_bucket **oldtable;

    oldsize  = ht->size;
    oldtable = ht->table;

    DBGF("n_hash_rehash %d\n", ht->size);

    newsize = 4 * ht->size;
    //if (ht->hash_fn)
    //    newsize += 1;

    ht->table = n_calloc(newsize, sizeof(*ht->table));
    if (ht->table == NULL) {
        ht->table = oldtable;
        return;
    }

    ht->size = newsize;
    ht->items = 0;

    for (i = 0; i < oldsize; i++) {
        register struct hash_bucket *node, *next_node;
        if (oldtable[i] == NULL)
            continue;

        for (node = oldtable[i]; node != NULL; node = next_node) {
            next_node = node->next;
            n_hash_put_node(ht, node);
        }
    }

    free(oldtable);
}


static
tn_hash *n_hash_put_node(tn_hash *ht, struct hash_bucket *node)
{
    struct hash_bucket *ptr;
    n_assert(node->klen > 0);

    unsigned khash = n_hash_compute_hash(ht, node->key, node->klen);

    ptr = ht->table[khash];
    if (ptr == NULL) {
        node->next = NULL;
        ht->items++;
        ht->table[khash] = node;
        return ht;
    }

    for (ptr = ht->table[khash]; NULL != ptr; ptr = ptr->next) {
        if (node->klen == ptr->klen && strcmp(node->key, ptr->key) == 0) {
            trurl_die("n_hash_put_node: key '%s' already in table\n", node->key);
            return NULL;
        }
    }

    node->next = ht->table[khash];
    ht->table[khash] = node;

    ht->items++;
    return ht;
}

static inline
struct hash_bucket *new_bucket(tn_hash *ht, const char *key, int klen)
{
    struct hash_bucket *ptr;
    int buflen = 0;

    n_assert(klen > 0);

    if ((ht->flags & TN_HASH_NOCPKEY) == 0)
        buflen = klen > 0 ? klen + 1 : (int)strlen(key) + 1;

    if (ht->na)
        ptr = ht->na->na_malloc(ht->na, sizeof(*ptr) + buflen);
    else
        ptr = n_malloc(sizeof(*ptr) + buflen);

    ptr->klen = klen;

    if (buflen == 0) {
        ptr->key = (char*)key;
    } else {
        memcpy(ptr->_buf, key, buflen);
        ptr->key = ptr->_buf;
    }

    return ptr;
}


#if 0
static
tn_hash *n_hash_put_ll(tn_hash *ht, const char *key, const void *data,
                       int replace)
{
    struct hash_bucket *ptr;
    unsigned val;
    int klen = strlen(key);

    if ((ht->flags & TN_HASH_REHASH) && ht->size - ht->items <= ht->size / 3)
        n_hash_rehash(ht);

    val = n_hash_compute_hash(ht, key, klen);
    ptr = ht->table[val];

    /* free slot */
    if (ptr == NULL) {
        ptr = new_bucket(ht, key, klen);
        ptr->next = NULL;
        ptr->data = (void *) data;
        ht->items++;
        ht->table[val] = ptr;
        return ht;
    }


    DBGF("INUSE %s, ", key);
    for (ptr = ht->table[val]; NULL != ptr; ptr = ptr->next) {
        DBGF("%s, ", ptr->key);
        if (strcmp(key, ptr->key) == 0) {
            if (!replace) {
                trurl_die("n_hash_insert: key '%s' already in table\n", key);
                return NULL;
            }

            if (ptr->data && ht->free_fn)
                ht->free_fn(ptr->data);
            ptr->data = (void *) data;
            return ht;
        }
    }
    DBGF("\n");

    ptr = new_bucket(ht, key, klen);
    ptr->data = (void *) data;
    ptr->next = ht->table[val];
    ht->table[val] = ptr;

    ht->items++;
    return ht;
}
#endif

/* find free slot around khash index */
static inline
uint32_t find_free_slot(const tn_hash *ht, const char *key, int klen, uint32_t khash)
{
    struct hash_bucket **tbl = ht->table;
    uint32_t shift = n_hash_nextslotv(key, klen);
    uint32_t nexthash = khash + shift;

    if (nexthash < ht->size && tbl[nexthash] == NULL)
        return nexthash;

    if (khash > shift) {
        nexthash = khash - shift;
        if (tbl[nexthash] == NULL)
            return nexthash;
    }

    return khash;
}

static
tn_hash *n_hash_put(tn_hash *ht, const char *key, int klen, unsigned khash,
                    const void *data, int replace)
{
    struct hash_bucket *ptr;

    n_assert(klen > 0);
    n_assert(klen < UINT16_MAX);

    if ((ht->flags & TN_HASH_REHASH) && ht->size - ht->items <= ht->size / 3) {
        n_hash_rehash(ht);
        khash = 0;              /* invalidate due to table size change */
    }

    if (!khash)
        khash = n_hash_compute_hash(ht, key, klen);

    if ((ht->flags & TN_HASH_NOREPLACE) == 0) {
        ptr = n_hash_get_bucket(ht, key, klen, khash);
        if (ptr) {
            if (!replace) {
                trurl_die("n_hash_insert: key '%s' %s already in table\n", key);
                return NULL;
            }

            if (ptr->data && ht->free_fn)
                ht->free_fn(ptr->data);

            ptr->data = (void *) data;
            return ht;
        }
    }

    ptr = ht->table[khash];
    if (ptr != NULL) {
        khash = find_free_slot(ht, key, klen, khash);
    }

    /* free slot */
    if (ptr == NULL) {
        ptr = new_bucket(ht, key, klen);
        ptr->next = NULL;
        ptr->data = (void *) data;
        ht->table[khash] = ptr;
        ht->items++;
        return ht;
    }

    DBGF("INUSE %s, ", key);
    for (ptr = ht->table[khash]; NULL != ptr; ptr = ptr->next) {
        DBGF("%s, ", ptr->key);
        if (strcmp(key, ptr->key) == 0) {
            trurl_die("n_hash_insert: key '%s' %s already in table\n",
                      key, ptr->key);
        }
    }
    DBGF("\n");

    ptr = new_bucket(ht, key, klen);
    ptr->data = (void *) data;
    ptr->next = ht->table[khash];
    ht->table[khash] = ptr;
    ht->items++;


    return ht;
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


tn_hash *n_hash_replace(tn_hash *ht, const char *key, const void *data)
{
    if (ht->flags & TN_HASH_NOREPLACE) {
        trurl_die("n_hash_replace: replace requested for"
                  " \"non-replace\" hash table");
        return NULL;
    }

    return n_hash_put(ht, key, strlen(key), 0, data, 1);
}

/* backward compat */
tn_hash *n_hash_insert_ex(tn_hash *ht, const char *key, int klen, unsigned khash,
                          const void *data) {
    return n_hash_hinsert(ht, key, klen, khash, data);
}
