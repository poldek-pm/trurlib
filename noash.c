/*
  TRURLib, open addressing hash table
*/

#include <string.h>
#include <stdlib.h>

#include "trurl_internal.h"
#include "nhash.h"
#include "noash.h"
#undef n_oash_keys

#include "nmalloc.h"

struct tentry {
    uint32_t hash;
    uint16_t psl;
    uint16_t klen;
    uint32_t data_index;
};

struct dentry {
    void    *data;
    char    *key;
    char    _buf[];
};

struct trurl_oash_table {
    uint16_t    _refcnt;
    uint16_t    flags;

    uint32_t size;
    struct tentry *table;
    uint32_t items;

    uint32_t dcap;
    struct dentry **data;
    uint32_t dsize;

    void          (*free_fn) (void *);
    tn_alloc      *na;
};

#define node_dentry(ht, node) ht->data[node->data_index]
#define tindex(size, hash) (hash) & (size - 1)
#define entry_key(ht, e) ((ht->data[(e)->data_index])->key)

inline int tentry_eq(struct tentry *e, struct tentry *e1) {
    return (e->hash == e1->hash && e->klen == e1->klen);
}

inline int tentry_samekey(const tn_oash *ht, struct tentry *e, struct tentry *e1)
{
    return (e->hash == e1->hash &&
            e->klen == e1->klen &&
            memcmp(ht->data[e->data_index]->key, ht->data[e1->data_index]->key, e->klen) == 0);
}

inline struct dentry *get_dentry(const tn_oash *ht, const struct tentry *e) {
    return ht->data[e->data_index];
}

static tn_oash *ohash_new_ex(tn_alloc *na,
                              size_t size, void (*freefn) (void *))
{
    tn_oash *ht = NULL;

    n_assert(size < UINT32_MAX);

    if (na == NULL)
        ht = n_calloc(1, sizeof(*ht));
    else
        ht = na->na_calloc(na, sizeof(*ht));

    if (ht == NULL)
        return NULL;

    ht->flags = 0;
    ht->table = n_calloc(size, sizeof(*ht->table));
    ht->data = n_calloc(size, sizeof(*ht->data));

    ht->size = size;
    ht->free_fn = freefn;
    ht->_refcnt = 0;
    ht->na = na;

    return ht;
}

static
tn_oash *ohash_new2(tn_alloc *na, size_t size, void (*freefn) (void *))
{
    register size_t rsize = 4;

    while (rsize < size)
        rsize <<= 1;
    return ohash_new_ex(na, rsize, freefn);
}

tn_oash *n_oash_new_ex(size_t size, void (*freefn) (void *),
                        unsigned int (*hashfn) (const char*))
{
    tn_oash *ht = NULL;

    if (hashfn == NULL)
        return ohash_new2(NULL, size, freefn);

    ht = ohash_new_ex(NULL, size, freefn);
    return ht;
}

tn_oash *n_oash_new_na(tn_alloc *na, size_t size, void (*freefn) (void *))
{
    if (na == NULL)
        na = n_alloc_new(16, TN_ALLOC_OBSTACK);
    else
        na = n_ref(na);

    return ohash_new2(na, size, freefn);
}

int n_oash_size(const tn_oash *ht)
{
    return ht->items;
}

static inline uint32_t murmur_32_scramble(uint32_t k) {
    k *= 0xcc9e2d51;
    k = (k << 15) | (k >> 17);
    k *= 0x1b873593;
    return k;
}

#define CDB_HASHSTART 5381
// Murmur hash was written by Austin Appleby, and is placed in the public
// domain. The author hereby disclaims copyright to this source code.
static inline uint32_t murmur3_hash(const uint8_t* key, int len)
{
    uint32_t h, k;

    h = CDB_HASHSTART ^ len;
    //h = 0;
    /* Read in groups of 4. */
    for (size_t i = len >> 2; i; i--) {
        // Here is a source of differing results across endiannesses.
        // A swap here has no effects on hash properties though.
        k = *((uint32_t*)key);
        key += sizeof(uint32_t);
        h ^= murmur_32_scramble(k);
        h = (h << 13) | (h >> 19);
        h = h * 5 + 0xe6546b64;
    }
    /* Read the rest. */
    k = 0;
    for (size_t i = len & 3; i; i--) {
        k <<= 8;
        k |= key[i - 1];
    }
    // A swap is *not* necessary here because the preceeding loop already
    // places the low bytes in the low places according to whatever endianess
    // we use. Swaps only apply when the memory is copied in a chunk.
    h ^= murmur_32_scramble(k);
    /* Finalize. */
    h ^= len;
    h ^= h >> 16;
    h *= 0x85ebca6b;
    h ^= h >> 13;
    h *= 0xc2b2ae35;
    h ^= h >> 16;

    return h;
}

uint32_t n_oash_compute_hash(const tn_oash *ht, const char *s, int len)
{
    (void)ht;
    n_assert(len > 0);
    //uint32_t v = djb_hash((const unsigned char *)s, len);
    uint32_t v = murmur3_hash((const unsigned char*)s, len);
    //uint32_t v = farmhash32((const unsigned char*)s, len);
    //uint32_t v = spookyhash32(s, len, 0);
    return v;
}

uint32_t n_oash_compute_hash_len(const tn_oash *ht, const char *s, int *slen)
{
    int len = strlen(s);

    if (slen)
        *slen = len;

    return n_oash_compute_hash(ht, s, len);
}

int n_oash_ctl(tn_oash *ht, unsigned int flags)
{
    if (ht->items > 0) {
        trurl_die("n_oash_ctl: hash table not empty");
        return 0;
    }
    ht->flags |= flags;
    return 1;
}

void n_oash_free(tn_oash *ht)
{
    size_t i;

    if (ht->_refcnt > 0) {
        ht->_refcnt--;
        return;
    }

    for (i = 0; i < ht->size; i++) {
        struct tentry *e = &ht->table[i];
        if (e->hash == 0)
            continue;

        struct dentry *de = ht->data[e->data_index];
        if (ht->free_fn != NULL && de && de->data != NULL) {
            ht->free_fn(de->data);
            if (!ht->na)
                n_free(de);
        }
    }

    free(ht->table);

    ht->table = NULL;
    ht->size = 0;
    ht->items = 0;

    if (ht->na == NULL)
        free(ht);
    else
        n_alloc_free(ht->na);
}

static inline
struct tentry *n_oash_put_entry(tn_oash *ht, struct tentry entry, struct dentry *de, int replace, int *replaced)
{
    register uint32_t size = ht->size;
    struct tentry *table = ht->table;
    struct tentry *e;

    n_assert(entry.klen > 0);
    n_assert(entry.hash != 0);

    entry.psl = 0;
    uint32_t index = tindex(size, entry.hash);

    e = &table[index];
    if (e->hash == 0) {
        ht->items++;
        table[index] = entry;
        return &table[index];
    }

    struct tentry *re = NULL;

    while (1) {
        entry.psl++;

        index = tindex(size, index + 1);
        e = &table[index];

        if (e->hash == 0) {
            ht->items++;
            table[index] = entry;
            if (re == NULL)
                re = &table[index];
            break;
        }

        if (de && tentry_eq(e, &entry) && strncmp(entry_key(ht, e), de->key, e->klen) == 0) {
            if (!replace) {
                trurl_die("key '%s' already in table\n", de->key);
                return NULL;
            }
            *replaced = 1;

            n_assert(re == NULL);
            return &table[index];
        }

        if (entry.psl > e->psl) { /* swap */
            struct tentry tmp = table[index];
            if (re == NULL)
                re = &table[index];

            table[index] = entry;
            entry = tmp;
        }
    }

    return re;
}

static
struct dentry *new_dentry(const tn_oash *ht, const char *key, int klen, const void *data)
{
    struct dentry *de;
    int buflen = 0;

    n_assert(klen > 0);

    if ((ht->flags & TN_HASH_NOCPKEY) == 0)
        buflen = klen > 0 ? klen + 1 : (int)strlen(key) + 1;

    if (ht->na)
        de = ht->na->na_malloc(ht->na, sizeof(*de) + buflen);
    else
        de = n_malloc(sizeof(*de) + buflen);

    //e->klen = klen;

    if (buflen == 0) {
        de->key = (char*)key;
    } else {
        memcpy(de->_buf, key, buflen);
        de->key = de->_buf;
    }

    de->data = (void *)data;
    return de;
}

static
void ohash_rehash();

static
tn_oash *n_oash_put(tn_oash *ht, const char *key, int klen, uint32_t khash,
                    const void *data, int replace)
{
    struct tentry e = {};

    n_assert(klen > 0);
    n_assert(klen < UINT16_MAX);

    if ((ht->flags & TN_HASH_REHASH) && ht->size - ht->items <= ht->size / 3) {
        ohash_rehash(ht);
    }

    if (ht->size == ht->items) {
        trurl_die("n_oash: table is full");
    }

    if (khash == 0)
        khash = n_oash_compute_hash(ht, key, klen);

    e.hash = khash;
    e.psl = 0;
    e.klen = klen;

    struct dentry *de = new_dentry(ht, key, klen, data);

    int replaced = 0;
    struct tentry *ie = n_oash_put_entry(ht, e, de, replace, &replaced);
    if (ie == NULL)
        return NULL;

    if (!replaced) {
        ht->data[ht->dsize] = de;
        ie->data_index = ht->dsize;
        ht->dsize++;
    } else {
        struct dentry *exising = get_dentry(ht, ie);
        if (exising->data && ht->free_fn)
            ht->free_fn(exising->data);

        exising->data = de->data;

        if (!ht->na)
            n_free(de);
    }

    return ht;
}

tn_oash *n_oash_insert(tn_oash *ht, const char *key, const void *data)
{
    return n_oash_put(ht, key, strlen(key), 0, data, 0);
}

tn_oash *n_oash_hinsert(tn_oash *ht, const char *key, int klen, unsigned khash,
                        const void *data)
{
    return n_oash_put(ht, key, klen, khash, data, 0);
}

tn_oash *n_oash_hreplace(tn_oash *ht, const char *key, int klen, unsigned khash,
                         const void *data)
{
    if (ht->flags & TN_HASH_NOREPLACE) {
        trurl_die("n_oash_replace: replace requested for"
                  " \"non-replace\" hash table");
        return NULL;
    }

    return n_oash_put(ht, key, klen, khash, data, 1);
}

tn_oash *n_oash_replace(tn_oash *ht, const char *key, const void *data)
{
    if (ht->flags & TN_HASH_NOREPLACE) {
        trurl_die("n_oash_replace: replace requested for"
                  " \"non-replace\" hash table");
        return NULL;
    }

    return n_oash_hreplace(ht, key, strlen(key), 0, data);
}

/* backward compat */
tn_oash *n_oash_insert_ex(tn_oash *ht, const char *key, int klen, unsigned khash,
                          const void *data) {
    return n_oash_hinsert(ht, key, klen, khash, data);
}

void n_oash_it_init(tn_oash_it *hi, tn_oash *ht)
{
    hi->ht = ht;
    hi->pos = 0;
}

void *n_oash_it_get(tn_oash_it *hi, const char **key) {
    struct tentry *table = hi->ht->table;
    size_t i = hi->pos;

    while (table[i].hash == 0 && i < hi->ht->size)
        i++;

    if (i >= hi->ht->size)
        return NULL;

    struct tentry *e = &table[i];
    struct dentry *d = hi->ht->data[e->data_index];

    hi->pos = i + 1;
    if (key)
        *key = d->key;

    return d->data;
}

static void ohash_rehash(tn_oash *ht)
{
    size_t i, newsize, oldsize;
    struct tentry **oldtable;

    oldsize  = ht->size;
    oldtable = &ht->table;

    DBGF("n_oash_rehash %d\n", ht->size);

    newsize = ht->size << 1;
    ht->table = n_calloc(newsize, sizeof(*ht->table));
    if (ht->table == NULL) {
        ht->table = *oldtable;
        return;
    }

    ht->data = n_realloc(ht->data, newsize);
    memset(&ht->data[ht->size], 0, (newsize - oldsize) * sizeof(*ht->data));
    ht->size = newsize;
    ht->items = 0;

    for (i = 0; i < oldsize; i++) {
        if (oldtable[i] == NULL)
            continue;

        n_oash_put_entry(ht, *oldtable[i], NULL, 0, NULL);
    }

    free(*oldtable);
}

static inline
int psldiff(const tn_oash *ht, const struct tentry *e, uint32_t index)
{
    uint32_t psl;
    uint32_t size = ht->size;
    uint32_t index0 = tindex(size, e->hash);

    if (index0 <= index) {
        psl = index - index0;
    } else {
        psl = index + (size - index0);
    }

    return psl;
}

static
struct tentry *ohash_get_entry(const tn_oash *ht,
                               const char *key, int klen, uint32_t khash,
                               uint32_t *bindex)
{
    struct tentry *table = ht->table;
    //struct tentry search = { khash, 0, klen }
    //n_assert(klen > 0);
    //n_assert(klen < UINT16_MAX);

    uint32_t size = ht->size;
    uint32_t index = tindex(size, khash);
    uint32_t i = index;
    uint32_t np = 0;

    struct tentry *e = &table[i];
    while (e->hash != 0) {
        uint32_t diff = psldiff(ht, e, i);

        //printf("get %s (%u): test [%d] %s (%u) psl%d diff%d\n", key, khash, i, ht->data[e->data_index]->key, e->hash, e->psl, diff);

        if (np > diff) {
            break;
        }

        if (i != index && e->psl == 0) {
            break;
        }

        np++;
        if (khash == e->hash && klen == e->klen &&
            strncmp(key, ht->data[e->data_index]->key, klen) == 0) {
            if (bindex)
                *bindex = i;

            return e;
        }

        i = tindex(size, i + 1);
        e = &table[i];
        //n_assert(i != index);
    }

    return NULL;
}

void *n_oash_get_ex(const tn_oash *ht,
                    const char *key, int *klen, unsigned *khash)
{
    struct tentry *e;

    *klen = strlen(key);
    *khash = n_oash_compute_hash(ht, key, *klen);

    e = ohash_get_entry(ht, key, *klen, *khash, NULL);
    return e ? get_dentry(ht, e)->data : NULL;
}


void *n_oash_get(const tn_oash *ht, const char *key)
{
    int klen;
    unsigned khash;

    return n_oash_get_ex(ht, key, &klen, &khash);
}

int n_oash_exists_ex(const tn_oash *ht, const char *key, int klen,
                     unsigned *khash)
{
    *khash = n_oash_compute_hash(ht, key, klen);
    return ohash_get_entry(ht, key, klen, *khash, NULL) != NULL;
}

int n_oash_exists(const tn_oash *ht, const char *key)
{
    int klen = strlen(key);
    unsigned khash;

    return n_oash_exists_ex(ht, key, klen, &khash);
}

/* h API */
int n_oash_hexists(const tn_oash *ht, const char *key, int klen, unsigned khash)
{
    return ohash_get_entry(ht, key, klen, khash, NULL) != NULL;
}

void *n_oash_hget(const tn_oash *ht, const char *key, int klen, unsigned khash)
{
    struct tentry *e = ohash_get_entry(ht, key, klen, khash, NULL);
    return e ? get_dentry(ht, e)->data : NULL;
}

tn_array *n_oash_keys_ext(const tn_oash *ht, int cp)
{
    register size_t i;
    tn_array *keys;
    tn_fn_free free_fn = NULL;

    if (cp)
        free_fn = n_free;

    keys = n_array_new(ht->items, free_fn, (tn_fn_cmp)strcmp);

    for (i = 0; i < ht->size; i++) {
        if (ht->table[i].hash == 0)
            continue;

        struct dentry *de = get_dentry(ht, &ht->table[i]);
        n_array_push(keys, cp ? n_strdup(de->key) : de->key);
    }

    return keys;
}

tn_array *n_oash_keys(const tn_oash *ht)
{
    return n_oash_keys_ext(ht, 0);
}

tn_array *n_oash_values(const tn_oash *ht)
{
    register size_t i;
    tn_array *values;

    values = n_array_new(ht->items, NULL, NULL);

    for (i = 0; i < ht->size; i++) {
        if (ht->table[i].hash == 0)
            continue;

        struct dentry *de = get_dentry(ht, &ht->table[i]);
        n_array_push(values, de->data);
    }

    return values;
}

void *n_oash_remove(tn_oash *ht, const char *key)
{
    int klen = strlen(key);
    uint32_t khash = n_oash_compute_hash(ht, key, klen);

    uint32_t eindex = 0;
    struct tentry *e = ohash_get_entry(ht, key, klen, khash, &eindex);
    if (!e)
        return NULL;

    n_assert(e->hash == khash);

    struct tentry ee = *e;
    uint32_t size = ht->size;
    uint32_t iprev = eindex;
    uint32_t i = tindex(size, eindex + 1);

    struct tentry *table = ht->table;
    e = &table[i];

    while (e->hash != 0 && e->psl > 0) {
        //n_assert(psldiff(ht, b, i) == b->psl);

        e->psl -= 1;
        table[iprev] = *e;

        iprev = i;
        i = tindex(size, i + 1);
        e = &table[i];
    }
    table[iprev].hash = 0;

    struct dentry *de = get_dentry(ht, &ee);
    void *data = de->data;
    if (!ht->na)
        n_free(de);

    ht->items -= 1;
    return data;
}

tn_oash *n_oash_dup(const tn_oash *ht, tn_fn_dup dup_fn)
{
    register size_t  i = 0;
    tn_oash          *h;

    h = n_oash_new(ht->size, NULL);
    h->size = ht->size;
    h->items = 0;
    h->free_fn = ht->free_fn;
    h->flags = h->flags;
    h->flags &= ~(TN_HASH_NOCPKEY);

    memcpy(h->table, ht->table, ht->size * sizeof(*h->table));
    for (i = 0; i < ht->size; i++) {
        if (ht->table[i].hash == 0)
            continue;

        struct tentry *e = &ht->table[i];

        struct dentry *de = get_dentry(ht, e);
        void *data = de->data;
        if (data && dup_fn)
            data = dup_fn(de->data);

        struct dentry *newde = new_dentry(ht, de->key, e->klen, data);
        h->data[e->data_index] = newde;
    }

    return h;
}

void n_oash_clean(tn_oash *ht)
{
    size_t i;
    for (i = 0; i < ht->size; i++) {
        struct tentry *e = &ht->table[i];
	if (e->hash) {
            e->hash = 0;

            struct dentry *de = get_dentry(ht, e);
            if (ht->free_fn != NULL && de->data != NULL)
		ht->free_fn(de->data);

            n_free(de);
            ht->data[e->data_index] = NULL;
        }
    }

    ht->items = 0;
}


static char store_version[] = "mr3";
static int store_version_len = 3;

int n_oash_store(const tn_oash *ht, tn_stream *st, int (*store_fn) (void *, tn_stream *, void*), void *store_ctx)
{
    register uint32_t i, n = 0;

    n_stream_write(st, store_version, store_version_len);
    n_stream_write_uint32(st, ht->size);

    uint16_t flags = ht->flags & ~(TN_HASH_NOCPKEY);
    n_stream_write_uint16(st, flags);
    n_stream_write_uint32(st, ht->items);

    for (i = 0; i < ht->size; i++) {
        struct tentry *e = &ht->table[i];
        if (e->hash == 0)
            continue;

        n_stream_write_uint32(st, i);
        n_stream_write_uint32(st, e->hash);
        n_stream_write_uint16(st, e->psl);
        n_stream_write_uint16(st, e->klen);
        n_stream_write_uint32(st, e->data_index);

        struct dentry *de = get_dentry(ht, e);
        n_stream_write(st, de->key, e->klen);

        if (!store_fn(de->data, st, store_ctx)) {
            return 0;
        }
        n++;
    }

    return n;
}

tn_oash *n_oash_restore(tn_stream *st,
                          void *(*restore_fn) (tn_stream *, void*), void *restore_ctx,
                          void (*freefn) (void *))
{
    char version[32] = {0};
    uint32_t size;

    if (!n_stream_read(st, &version, store_version_len))
        return NULL;
    version[store_version_len] = '\0';

    if (memcmp(version, store_version, store_version_len) != 0)
        return NULL;

    n_stream_read_uint32(st, &size);
    tn_oash *ht = n_oash_new(size, freefn);
    n_stream_read_uint16(st, &ht->flags);
    n_stream_read_uint32(st, &ht->items);

    for (uint32_t i = 0; i < ht->items; i++) {
        uint32_t index = 0;
        char key[512];

        n_stream_read_uint32(st, &index);

        struct tentry *e = &ht->table[index];

        n_stream_read_uint32(st, &e->hash);
        n_stream_read_uint16(st, &e->psl);
        n_stream_read_uint16(st, &e->klen);
        n_stream_read_uint32(st, &e->data_index);

        n_stream_read(st, key, e->klen);
        key[e->klen] = '\0';

        void *data = restore_fn(st, restore_ctx);

        struct dentry *de = new_dentry(ht, key, e->klen, data);
        ht->data[e->data_index] = de;
    }

    ht->free_fn = freefn;
    return ht;
}

int n_oash_map(const tn_oash *ht, void (*map_fn) (const char *, void *))
{
    register size_t i, n = 0;
    register struct tentry *tmp;

    for (i = 0; i < ht->size; i++) {
        if (ht->table[i].hash == 0)
            continue;

        tmp = &ht->table[i];
        struct dentry *de = get_dentry(ht, tmp);
        map_fn(de->key, de->data);
        n++;
    }

    return n;
}

int n_oash_map_arg(const tn_oash *ht,
		   void (*map_fn) (const char *, void *, void *),
		   void *arg)
{
    register size_t i, n = 0;
    register struct tentry *tmp;

    for (i = 0; i < ht->size; i++) {
        tmp = &ht->table[i];

        if (tmp->hash == 0)
            continue;

        struct dentry *de = get_dentry(ht, tmp);
        map_fn(de->key, de->data, arg);
        n++;
    }

    return n;
}

int n_oash_dump(const tn_oash *ht)
{
    uint32_t i, n = 0;

    printf("DUMP %p size=%d\n", ht, ht->items);
    for (i = 0; i < ht->size; i++) {
        struct tentry *e = &ht->table[i];
        struct dentry *d = get_dentry(ht, e);

        if (e && e->hash > 0) {
            unsigned diff = psldiff(ht, e, i);
            printf("[%u] %u. h %u, psl %d (%d)%s: %s => %p\n", i, n, e->hash, e->psl, diff,
                   diff != e->psl ? " DIFFER" : "", d->key, d->data);
            n++;
        } else {
            printf("[%u] empty\n", i);
        }
    }

    return n;
}

int n_oash_stats(const tn_oash *ht)
{
    register size_t i, n = 0;
    int nempts = 0;
    int pslsum = 0;
    int empty_len_sum = 0;
    int empty_blocks = 0;
    int empty_len = 0;
    int maxpsl = 0;

    int nonempty_len = 0;
    int nonempty_blocks = 0;

    for (i = 0; i < ht->size; i++) {
        if (i > 0) {
            if (ht->table[i-1].hash && ht->table[i].hash == 0) {
                nonempty_blocks++;
                nonempty_len = 0;
            } else if (ht->table[i-1].hash && ht->table[i].hash != 0) {
                empty_blocks++;
                empty_len_sum += empty_len;
                empty_len = 0;
            }
        }

        if (ht->table[i].hash == 0) {
            nempts++;
            empty_len++;
            continue;
        } else {
            nonempty_len++;
        }

        pslsum += ht->table[i].psl;
        if (ht->table[i].psl > maxpsl)
            maxpsl = ht->table[i].psl;
    }

    printf("ht(%p): %u keys, %u slots (%.2f%% empty, avg empty len %.lf), avg psl %.1lf, max psl %d)\n",
           ht,
           ht->items,
           ht->size,
           (100.0 * nempts)/ht->size,
           empty_len_sum ? empty_len_sum / (1.0 * empty_blocks) : 0,
           pslsum > 0 ? pslsum/(ht->items * 1.0) : 0,
           maxpsl);
    return n;
}
