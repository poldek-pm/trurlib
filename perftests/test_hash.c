/*
   $Id$
*/

#include <errno.h>
#define __USE_POSIX2
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include "nhash.h"
#include "noash.h"
#include "nmalloc.h"

#include <sys/time.h>
#include <sys/resource.h>

double get_time() {
    struct timeval t;
    gettimeofday(&t, NULL);
    return t.tv_sec + t.tv_usec*1e-6;
}

#define decl_timer clock_t c0 = 0; double t0 = 0;
#define start_timer c0 = clock(); t0 = get_time();
#define stop_timer(name, label) printf("%-8s %-8s %fs e%fs\n", name, label, (double)(clock() - c0) / CLOCKS_PER_SEC, get_time() - t0);

struct str {
    int len;
    char s[];
};

tn_array *load_keys(int limit, int dedup) {
    char buf[1024];

#if 0
    char *foo = strdup("foo");
    printf("sizeof %ld\n", sizeof(struct str));

    struct str *st = malloc(sizeof(*st) + sizeof(char*));
    st->len = 3;
    memcpy(st->s, &foo, sizeof(foo));
    printf("s %d, %p %p %s\n", sizeof(foo), foo, st->s, *(char **)(st->s));
    foo[1] = 'b';
    printf("s %d, %p %p %s\n", sizeof(foo), foo, st->s, *(char **)(st->s));

    printf("s %p %p %s\n", foo, st->s, st->s);
#endif
    tn_array *keys = n_array_new(25 * 4096, free, NULL);

    FILE *stream;
    if (!dedup)
        stream = popen("rpm -qaRl --provides | cut -f1 -d' '", "r");
    else
        stream = popen("rpm -qaRl --provides | cut -f1 -d' ' | tr '[:upper:]' '[:lower:]' | sort -u", "r");

    while (fgets(buf, sizeof(buf), stream)) {
        if (limit > 0 && n_array_size(keys) > limit) {
            break;
        }

        int len = strlen(buf);
        if (len < 4) {
            continue;
        }

        len--;
        buf[len] = '\0'; // \n
        struct str *st = malloc(sizeof(*st) + len + 1);
        st->len = len;
        strcpy((char *)st->s, buf);
        n_array_push(keys, st);
    }
    pclose(stream);

    printf("Loaded %d keys\n", n_array_size(keys));

    return keys;
}

static bool dump_stats = false;

struct hash_table_i {
    const char *name;
    void *(*init) (int);
    void *(*put) (void *, const char *, const char *);
    const char *(*get) (void *, const char *);
    void **(*getins) (void *, const char *, size_t);
    void (*destroy)(void *);
};

#define NAME vstrmap
#define KEY_TY char*
#define VAL_TY char*
#include "v/verstable.h"

void *vtt_init(int size) {
    vstrmap *h = malloc(sizeof(*h));
    vstrmap_init(h);
    vstrmap_reserve(h, size);

    return h;
}

void *vtt_put(void *map, const char *k, const char *v) {
    vstrmap *h = map;
    vstrmap_insert(h, (char*)k, (char *)v);
    return h;
}

const char *vtt_get(void *map, const char *k) {
    vstrmap *h = map;
    //printf("vtget %s\n", k);
    vstrmap_itr itr = vstrmap_get(h, (char *)k);
    //printf("   -> %s = %s\n", k, itr.data->val);
    if (vt_is_end(itr))
        return NULL;

    return itr.data->val;
}

void **vtt_getins(void *map, const char *k, size_t len) {
    vstrmap *h = map;
    (void)len;
    vstrmap_itr itr = vt_get_or_insert(h, (char *)k, NULL);
    if (vt_is_end(itr))
        return NULL;

    return (void **)&itr.data->val;
}

void vtt_destroy(void *map) {
    if (dump_stats)
        printf("vt(%p) %lu keys, %lu slots\n", map, vstrmap_size(map), vstrmap_bucket_count(map));

    vstrmap_cleanup(map);
    free(map);
}

struct hash_table_i ht_verstable = { "vt", vtt_init, vtt_put, vtt_get, vtt_getins, vtt_destroy };

void *nh_init(int size) {
    tn_alloc *na = n_alloc_new(4, TN_ALLOC_OBSTACK);
    tn_hash *h = n_hash_new_na(na, size, NULL);
    n_hash_ctl(h, TN_HASH_NOCPKEY | TN_HASH_REHASH);
    return h;
}

void *nh_put(void *map, const char *k, const char *v) {
    return n_hash_insert(map, k, v);
}

const char *nh_get(void *map, const char *k) {
    return n_hash_get(map, k);
}

void nh_destroy(void *map) {
    if (dump_stats)
        n_hash_stats(map);
    n_hash_free(map);
}
struct hash_table_i ht_nhash = { "nhash", nh_init, nh_put, nh_get, NULL, nh_destroy };

void *noh_init(int size) {
    tn_alloc *na = n_alloc_new(512, TN_ALLOC_OBSTACK);
    tn_oash *h = n_oash_new_na(na, size, NULL);

    n_oash_ctl(h, TN_HASH_NOCPKEY | TN_HASH_REHASH);
    return h;
}

void *noh_put(void *map, const char *k, const char *v) {
    return n_oash_insert(map, k, v);
}

const char *noh_get(void *map, const char *k) {
    return n_oash_get(map, k);
}

void **noh_getins(void *map, const char *k, size_t len) {
    //printf("getins %s?\n", k);
    void **re = n_oash_get_insert(map, k, len);
    //printf("getins %s %p\n", k, re);
    return re;
}

void noh_destroy(void *map) {
    if (dump_stats)
        n_oash_stats(map);
    n_oash_free(map);
}

struct hash_table_i ht_noash = { "noash", noh_init, noh_put, noh_get, noh_getins, noh_destroy };

void bench_hash_table(struct hash_table_i *t, tn_array *keys, int limit) {
    decl_timer;
    //tn_alloc *na = n_alloc_new(4, TN_ALLOC_OBSTACK);
    //tn_hash *h = n_hash_new_na(na, htsize, NULL);
    void *h = t->init(limit * 1.4);
    start_timer;
    for (int i = 0; i < n_array_size(keys) && i < limit; i++) {
        struct str *st = n_array_nth(keys, i);
        t->put(h, st->s, st->s);
    }
    stop_timer(t->name, "put");


    /* restart, to test getins */
    t->destroy(h);
    h = t->init(limit * 1.4);

    if (t->getins) {
        start_timer;
        for (int i = 0; i < n_array_size(keys) && i < limit; i++) {
            struct str *st = n_array_nth(keys, i);
            const char **v = (const char **)t->getins(h, st->s, st->len);
            n_assert(v);
            if (*v == NULL)
                *v = st->s;
            else
                n_assert(*v == st->s);
        }
        stop_timer(t->name, "getins");
    }

    start_timer;
    for (int i = 0; i < n_array_size(keys) && i < limit; i++) {
        struct str *st = n_array_nth(keys, i);
        const char *v = t->get(h, st->s);
        (void)v;
    }
    stop_timer(t->name, "get");

    start_timer;
    for (int i = 0; i < n_array_size(keys) && i < limit; i++) {
        struct str *st = n_array_nth(keys, i);
        char c = st->s[0];
        st->s[0] = 'X';
        const char *v = t->get(h, st->s);
        (void)v;

        st->s[0] = c;
    }
    stop_timer(t->name, "getne");
    t->destroy(h);
}

void test_hash() {
    int nn = 32 * 1024;
    char *keys[4*nn];
    tn_oash *h;

    for (int i = 0; i < 4*nn; i++) {
        char key[64];
        snprintf(key, sizeof(key), "key%02d", i);
        keys[i] = strdup(key);
    }

    h = n_oash_new(nn, NULL);
    n_oash_ctl(h, TN_HASH_NOCPKEY | TN_HASH_REHASH);

    for (int i = 0; i < 4*nn; i++) {
        const char *key = keys[i];
        void **data = n_oash_get_insert(h, key, strlen(key));
        if (*data == NULL)
            *data = (char *)key;
        else
            printf("k %s already inserted\n", key);
    }

    for (int i = 0; i < 4*nn; i++) {
        free(keys[i]);
    }

    n_oash_free(h);
}

void test_hash_get_insert(tn_array *keys) {
    tn_oash *h;
    h = n_oash_new(n_array_size(keys), NULL);
    n_oash_ctl(h, TN_HASH_NOCPKEY | TN_HASH_REHASH);

    for (int i = 0; i < n_array_size(keys); i++) {
        const struct str *s = n_array_nth(keys, i);
        void **data = n_oash_get_insert(h, s->s, s->len);
        n_assert(data);

        if (*data == NULL)
            *data = (char *)s->s;
        else
            n_assert(memcmp(*data, s->s, s->len) == 0);
    }

    /* second loop - all keys already in */
    for (int i = 0; i < n_array_size(keys); i++) {
        const struct str *s = n_array_nth(keys, i);
        void **data = n_oash_get_insert(h, s->s, s->len);
        n_assert(data);
        n_assert(*data != NULL);
        n_assert(memcmp(*data, s->s, s->len) == 0);
    }

    n_oash_free(h);
}


int main()
{
    tn_array *keys;

    keys = load_keys(0, 0);
    test_hash_get_insert(keys);
    n_array_free(keys);

    struct hash_table_i *hash_tables[] = { &ht_nhash, &ht_noash, &ht_verstable, NULL };

    keys = load_keys(0, 1);

    for (int i = (1<<6); i < (1<<9); i = i<<1) {
        int nkeys = 1024 * i;
        if (n_array_size(keys) < nkeys)
            nkeys = n_array_size(keys);

        printf("== %.0fk keys\n", nkeys/1024.0);
        int n = 0;
        while (hash_tables[n]) {
            bench_hash_table(hash_tables[n], keys, nkeys);
            n++;
        }
    }
}
