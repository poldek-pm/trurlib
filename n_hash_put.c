/* $Id$ */

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
    if (ht->hash_fn)
        newsize += 1;
    
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
    unsigned val = n_hash_dohash(ht, node->key, NULL);
    
    
    ptr = ht->table[val];
    if (ptr == NULL) {
        node->next = NULL;
        ht->items++;
        ht->table[val] = node;
        return ht;
    }

    for (ptr = ht->table[val]; NULL != ptr; ptr = ptr->next) {
        if (strcmp(node->key, ptr->key) == 0) {
            trurl_die("n_hash_put_node: key '%s' already in table\n", node->key);
            return NULL;
        }
    }
    
    node->next = ht->table[val];
    ht->table[val] = node;

    ht->items++;
    return ht;
}

static inline
struct hash_bucket *new_bucket(tn_hash *ht, const char *key, int klen)
{
    struct hash_bucket *ptr;
    int key_len = 0;
    

    if ((ht->flags & TN_HASH_NOCPKEY) == 0)
        key_len = klen > 0 ? klen + 1 : (int)strlen(key) + 1;
    
    if ((ptr = n_malloc(sizeof(*ptr) + key_len)) == NULL)
        return NULL;
    
    if (key_len == 0) {
        ptr->key = (char*)key;
        
    } else {
        memcpy(ptr->_buf, key, key_len);
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
    int klen;

    if ((ht->flags & TN_HASH_REHASH) && ht->size - ht->items <= ht->size / 3)
        n_hash_rehash(ht);

    val = n_hash_dohash(ht, key, &klen);
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

static
tn_hash *n_hash_put(tn_hash *ht, const char *key, const void *data, int replace)
{
    struct hash_bucket *ptr;
    unsigned val;
    int klen;
    
    if ((ht->flags & TN_HASH_REHASH) && ht->size - ht->items <= ht->size / 3)
        n_hash_rehash(ht);

    val = n_hash_dohash(ht, key, &klen);

    if ((ht->flags & TN_HASH_NOREPLACE) == 0) {
        ptr = n_hash_get_bucket(ht, key, klen, val);
        if (ptr) {
            if (!replace) {
                trurl_die("n_hash_insert: key '%s' %s already in table\n",
                          key, ptr->key);
                return NULL;
            }
            
            if (ptr->data && ht->free_fn)
                ht->free_fn(ptr->data);
            ptr->data = (void *) data;
            return ht;
        }
    }

    ptr = ht->table[val];
    if (ptr != NULL && ptr->next) {
        unsigned nextval = val + n_hash_nextslotv(key, klen);
        if (nextval < ht->size) {
            if ((ptr = ht->table[nextval]) == NULL) {
                DBGF("%d. nextmove(%d) %s[%d]\n", moves++, val, key, klen);
                val = nextval;
            }
        }
    }
    
    /* free slot */
    if (ptr == NULL) {
        ptr = new_bucket(ht, key, klen);
        ptr->next = NULL;
        ptr->data = (void *) data;
        ht->table[val] = ptr;
        ht->items++;
        return ht;
    }

    DBGF("INUSE %s, ", key);
    for (ptr = ht->table[val]; NULL != ptr; ptr = ptr->next) {
        DBGF("%s, ", ptr->key);
        if (strcmp(key, ptr->key) == 0) {
            trurl_die("n_hash_insert: key '%s' %s already in table\n",
                      key, ptr->key);
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

tn_hash *n_hash_insert(tn_hash *ht, const char *key, const void *data)
{
    return n_hash_put(ht, key, data, 0);
}

tn_hash *n_hash_replace(tn_hash *ht, const char *key, const void *data)
{
    if (ht->flags & TN_HASH_NOREPLACE) {
        trurl_die("n_hash_replace: replace requested for"
                  " \"non-replace\" hash table");
        return NULL;
    }
    return n_hash_put(ht, key, data, 1);
}
