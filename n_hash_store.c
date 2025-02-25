#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include "nstream.h"
#include "nhash.h"
#include "n_hash_int.h"

static char store_version[] = "mr3";
static int store_version_len = 3;

int n_hash_store(const tn_hash *ht, tn_stream *st, int (*store_fn) (void *, tn_stream *, void*), void *store_ctx)
{
    register uint32_t i, n = 0;

    n_stream_write(st, store_version, store_version_len);
    n_stream_write_uint16(st, ht->flags);
    n_stream_write_uint32(st, ht->size);
    n_stream_write_uint32(st, ht->items);

    for (i = 0; i < ht->size; i++) {
        register struct hash_bucket *b = ht->table[i];
        if (b == NULL)
            continue;

        n_stream_write_uint32(st, i);
        n_stream_write_uint32(st, b->hash);
        n_stream_write_uint16(st, b->psl);
        n_stream_write_uint16(st, b->klen);
        n_stream_write(st, b->key, b->klen);

        if (!store_fn(b->data, st, store_ctx)) {
            return 0;
        }
        n++;
    }

    return n;
}

tn_hash *n_hash_restore(tn_stream *st, void *(*restore_fn) (tn_stream *, void*), void *restore_ctx, void (*freefn) (void *))
{
    char version[32] = {0};
    struct trurl_hash_table sht = {0};

    if (!n_stream_read(st, &version, store_version_len))
        return NULL;
    version[store_version_len] = '\0';

    if (memcmp(version, store_version, store_version_len) != 0)
        return NULL;

    n_stream_read_uint16(st, &sht.flags);
    n_stream_read_uint32(st, &sht.size);
    n_stream_read_uint32(st, &sht.items);

    tn_hash *ht = n_hash_new(sht.size, NULL);

    for (uint32_t i = 0; i < sht.items; i++) {
        uint32_t index = 0;
        uint32_t hash = 0;
        uint16_t psl = 0;
        uint16_t klen = 0;
        char key[312];

        n_stream_read_uint32(st, &index);
        n_stream_read_uint32(st, &hash);
        n_stream_read_uint16(st, &psl);
        n_stream_read_uint16(st, &klen);
        n_stream_read(st, key, klen);
        key[klen] = '\0';

        struct hash_bucket *e = n_hash_new_bucket(ht, key, klen, hash);
        e->psl = psl;
        ht->table[index] = e;
        ht->items++;

        e->data = restore_fn(st, restore_ctx);
        if (!e->data) {
            n_hash_free(ht);
            ht = NULL;
            break;
        }
    }

    return ht;
}
