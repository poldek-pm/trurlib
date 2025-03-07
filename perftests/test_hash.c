/*
   $Id$
*/

#include <errno.h>
#define __USE_POSIX2
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

tn_array *KEYS;

struct str {
    int len;
    char s[];
};

int load_KEYS(int limit) {
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
    KEYS = n_array_new(25 * 4096, free, NULL);

    FILE *stream = popen("rpm -qaRl --provides | cut -f1 -d' ' | sort -u", "r");

    while (fgets(buf, sizeof(buf), stream)) {
        if (limit > 0 && n_array_size(KEYS) > limit) {
            break;
        }

        int len = strlen(buf);
        if (len < 3) {
            continue;
        }
        buf[len-1] = '\0'; // \n
        struct str *st = malloc(sizeof(*st) + len + 1);
        st->len = len;
        strcpy((char *)st->s, buf);
        n_array_push(KEYS, st);
    }
    pclose(stream);

    printf("Loaded %d keys\n", n_array_size(KEYS));

    return n_array_size(KEYS);
}

struct hash_table_i {
    const char *name;
    void *(*init) (int);
    void *(*put) (void *, const char *, const char *);
    const char *(*get) (void *, const char *);
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

void vtt_destroy(void *map) {
    vstrmap_cleanup(map);
    free(map);
}

struct hash_table_i ht_verstable = { "vt", vtt_init, vtt_put, vtt_get, vtt_destroy };

void *nh_init(int size) {
    tn_hash *h = n_hash_new(size, NULL);
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
    n_hash_stats(map);
    n_hash_free(map);
}
struct hash_table_i ht_nhash = { "nhash", nh_init, nh_put, nh_get, nh_destroy };

void *noh_init(int size) {
    tn_oash *h = n_oash_new(size, NULL);
    n_oash_ctl(h, TN_HASH_NOCPKEY | TN_HASH_REHASH);
    return h;
}

void *noh_put(void *map, const char *k, const char *v) {
    return n_oash_insert(map, k, v);
}

const char *noh_get(void *map, const char *k) {
    return n_oash_get(map, k);
}

void noh_destroy(void *map) {
    n_oash_stats(map);
    n_oash_free(map);
}

struct hash_table_i ht_noash = { "noash", noh_init, noh_put, noh_get, noh_destroy };

void bench_hash_table(struct hash_table_i *t, int limit) {
    decl_timer;
    //tn_alloc *na = n_alloc_new(4, TN_ALLOC_OBSTACK);
    //tn_hash *h = n_hash_new_na(na, htsize, NULL);
    void *h = t->init(limit * 2);
    start_timer;
    for (int i = 0; i < n_array_size(KEYS) && i < limit; i++) {
        struct str *st = n_array_nth(KEYS, i);
        t->put(h, st->s, st->s);
    }
    stop_timer(t->name, "put");

    start_timer;
    for (int i = 0; i < n_array_size(KEYS) && i < limit; i++) {
        struct str *st = n_array_nth(KEYS, i);
        const char *v = t->get(h, st->s);
        (void)v;
    }
    stop_timer(t->name, "get");

    start_timer;
    const char *nekey = "foo-bar";
    for (int i = limit; i < 2*limit; i++) {
        const char *v = t->get(h, nekey);
        (void)v;
    }
    stop_timer(t->name, "getne");
    t->destroy(h);
}



int main()
{
    struct hash_table_i *hash_tables[] = { &ht_nhash, &ht_noash, &ht_verstable, NULL };

    load_KEYS(0);

    for (int i = (1<<2); i < (1<<12); i = i<<2) {
        int nkeys = 1024 * i;
        if (n_array_size(KEYS) < nkeys)
            nkeys = n_array_size(KEYS);

        printf("== %d keys\n", nkeys);
        int n = 0;
        while (hash_tables[n]) {
            bench_hash_table(hash_tables[n], nkeys);
            n++;
        }
    }
}
