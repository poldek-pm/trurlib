#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include "n_hash_int.h"

static tn_hash *do_n_hash_new_ex(tn_alloc *na,
                                 size_t size, void (*freefn) (void *))
{
    tn_hash *ht = NULL;

    n_assert(size < UINT32_MAX);

    if (na == NULL)
        ht = n_calloc(1, sizeof(*ht));
    else
        ht = na->na_calloc(na, sizeof(*ht));

    if (ht == NULL)
        return NULL;

    ht->flags = 0;
    ht->table = n_calloc(size, sizeof(*ht->table));

    ht->size = size;
    ht->free_fn = freefn;
    ht->_refcnt = 0;
    ht->na = na;

    return ht;
}

static
tn_hash *n_hash_new2(tn_alloc *na, size_t size, void (*freefn) (void *))
{
    register size_t rsize = 4;

    while (rsize < size)
        rsize <<= 1;
    return do_n_hash_new_ex(na, rsize, freefn);
}

tn_hash *n_hash_new_ex(size_t size, void (*freefn) (void *),
                       unsigned int (*hashfn) (const char*))
{
    tn_hash *ht = NULL;

    if (hashfn == NULL)
        return n_hash_new2(NULL, size, freefn);

    ht = do_n_hash_new_ex(NULL, size, freefn);
    return ht;
}

tn_hash *n_hash_new_na(tn_alloc *na, size_t size, void (*freefn) (void *))
{
    if (na == NULL)
        na = n_alloc_new(16, TN_ALLOC_OBSTACK);
    else
        na = n_ref(na);

    return n_hash_new2(na, size, freefn);
}

int n_hash_size(const tn_hash *ht)
{
    return ht->items;
}

#define CDB_HASHSTART 5381
static inline uint32_t djb_hash(const unsigned char *s, int *slen)
{
    register uint32_t v = CDB_HASHSTART;
    const unsigned char *ss = s;

    while (*s) {
        v += (v << 5);
        v ^= *s;
        s++;
    }

    if (slen)
        *slen = s - ss;

    return v;
}

static inline uint32_t murmur_32_scramble(uint32_t k) {
    k *= 0xcc9e2d51;
    k = (k << 15) | (k >> 17);
    k *= 0x1b873593;
    return k;
}

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

uint32_t n_hash_compute_hash(const tn_hash *ht, const char *s, int len)
{
    (void)ht;
    n_assert(len > 0);
    //uint32_t v = djb_hash((const unsigned char *)s, &len);
    uint32_t v = murmur3_hash((const unsigned char*)s, len);
    return v;
}

uint32_t n_hash_compute_hash_len(const tn_hash *ht, const char *s, int *slen)
{
    int len = strlen(s);

    if (slen)
        *slen = len;

    return n_hash_compute_hash(ht, s, len);
}

uint32_t n_hash_compute_raw_hash(const char *s, int len) {
    n_assert(len > 0);
    return murmur3_hash((const unsigned char*)s, len);
}

uint32_t n_hash_compute_index_hash(const tn_hash *ht, uint32_t raw_hash) {
    return raw_hash & (ht->size - 1);
}
