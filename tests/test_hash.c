/*
   $Id$
*/

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>

#include "n_check.h"
#include "n_snprintf.h"
#include "nhash.h"
#include "nstream.h"
#include "n_hash_int.h"

// fail_unless macro woes
#pragma GCC diagnostic ignored "-Wformat-extra-args"

#define printf_clock(t0, label) printf("%s %fs\n", label, (double)(clock() - t0) / CLOCKS_PER_SEC);

tn_array *KEYS;

struct str {
    int len;
    char s[];
};


#if 0                           /* copy char* pointer to s[] */
void teststh() {
    char *foo = strdup("foo");


    printf("sizeof %ld\n", sizeof(struct str));

    struct str *st = malloc(sizeof(*st) + sizeof(char*));
    st->len = 3;
    memcpy(st->s, &foo, sizeof(foo));
    printf("s %d, %p %p %s\n", sizeof(foo), foo, st->s, *(char **)(st->s));
    foo[1] = 'b';
    printf("s %d, %p %p %s\n", sizeof(foo), foo, st->s, *(char **)(st->s));
    printf("s %p %p %s\n", foo, st->s, st->s);
}
#endif

int load_KEYS(int limit) {
    char buf[1024];

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
    //fclose(stream);

    printf("Loaded %d keys\n", n_array_size(KEYS));

    return n_array_size(KEYS);
}


void hash_map_func(const char *key, void *data)
{
    char *k = (char *) key;
    char *d = (char *) data;
    printf("'%s' = '%s'\n", k, d);
}

int n_hash_dump(const tn_hash *ht);

START_TEST(test_hash_base)
{
    int nn = 1024;
    char *keys[nn + 10];
    tn_hash *h;

    for (int i = 0; i < nn + 10; i++) {
        char key[64];
        snprintf(key, sizeof(key), "key%02d", i);
        keys[i] = strdup(key);
    }

    h = n_hash_new(nn, free);
    n_hash_ctl(h, TN_HASH_NOCPKEY);
    //n_hash_ctl(h, TN_HASH_REHASH);

    for (int i = 0; i < nn; i++) {
        const char *key = keys[i];
        n_hash_insert(h, key, strdup(key));
        //n_hash_dump(h);
        fail_unless(n_hash_exists(h, key), "%s: no such key", key);
    }
    expect_int(n_hash_size(h), nn);
    //n_hash_dump(h);

    for (int i = 0; i < nn + 10; i++) {
        const char *key = keys[i];
        const char *v = n_hash_get(h, key);
        if (i < nn)
            fail_unless(v && strcmp(v, key) == 0, "%s: no such key", key);
        else
            fail_if(n_hash_exists(h, key), "%s: should not be found", key);
            //n_assert(v == NULL);
    }

    //n_hash_dump(h);
    for (int i = 0; i < nn; i++) {
        const char *key = keys[i];
        const char *v = n_hash_remove(h, key);
        if (i < nn) {
            fail_if(n_hash_exists(h, key), "%s: should not be found", key);
            fail_unless(strcmp(v, key) == 0, "%s: should eq %s", key, v);
            //n_assert(strcmp(v, key) == 0);
        }

        //n_hash_dump(h);

        for (int j = 0; j < i+1; j++) {
            const char *s = keys[j];
            //int e = n_hash_exists(h, s);
            //n_assert(e == 0);
            fail_if(n_hash_exists(h, s), "%s: should not be found", key);
        }

        for (int j = i+1; j < nn; j++) {
            const char *s = keys[j];
            //int e = n_hash_exists(h, s);
            //n_assert(e == 1);
            fail_unless(n_hash_exists(h, s), "%s: should be found", key);
        }
    }
    expect_int(n_hash_size(h), 0);
}
END_TEST

int store_str(void *str, tn_stream *st, void *ctx) {
    const char *s = str;
    (void)ctx;
    if (!n_stream_write_uint16(st, strlen(s)))
        return 0;

    if (!n_stream_write(st, s, strlen(s)))
        return 0;

    return 1;
}

void *restore_str(tn_stream *st, void *ctx) {
    uint16_t len = 0;
    (void)ctx;
    if (!n_stream_read_uint16(st, &len)) {
        return NULL;
    }

    char *s = n_malloc(len + 1);
    n_stream_read(st, s, len);
    s[len] = '\0';

    return s;
}

static char *array_str(const tn_array *a)
{
    int size = n_array_size(a) * 128;
    char *buf = n_malloc(size);
    int n = 0;
    for (int i = 0; i < n_array_size(a); i++) {
        const char *s = (const char *)n_array_nth(a, i);
        n += n_snprintf(&buf[n], size - n, "%s,", s);
    }
    if (n > 0)
        buf[n - 1] = '\0';      /* trim last comma */

    return buf;
}

START_TEST(test_hash_store)
{
    int nn = 32;
    char *keys[nn + 10];
    tn_hash *h;

    for (int i = 0; i < nn + 10; i++) {
        char key[64];
        snprintf(key, sizeof(key), "key%02d", i);
        keys[i] = strdup(key);
    }

    h = n_hash_new(nn, free);
    n_hash_ctl(h, TN_HASH_NOCPKEY);
    //n_hash_ctl(h, TN_HASH_REHASH);

    for (int i = 0; i < nn; i++) {
        const char *key = keys[i];
        n_hash_insert(h, strdup(key), strdup(keys[(i + i/2) % nn]));
        //n_hash_dump(h);
        fail_unless(n_hash_exists(h, key), "%s: no such key", key);
    }
    expect_int(n_hash_size(h), nn);
#ifdef TN_HASH_WITH_STORE
    const char *path = NTEST_TMPPATH("hash.bin.gz");
    //printf("pp %s\n", path);
    //n_hash_dump(h);

    tn_stream *st = n_stream_open(path, "w", TN_STREAM_UNKNOWN);
    n_hash_store(h, st, store_str, NULL);
    n_stream_close(st);

    st = n_stream_open(path, "r", TN_STREAM_UNKNOWN);
    tn_hash *rh = n_hash_restore(st, restore_str, NULL, free);
    n_stream_close(st);
    fail_if(rh == NULL, "n_hash_restore should succeed");
    //n_hash_dump(rh);
    fail_if(n_hash_size(rh) != n_hash_size(h), "n_hash_restore: sizes should be equal");

    char *p, *q;
    p = array_str(n_hash_values(h));
    q = array_str(n_hash_values(rh));

    fail_unless(strcmp(p, q) == 0, "values are not equal");

    p = array_str(n_hash_keys(h));
    q = array_str(n_hash_keys(rh));
    fail_unless(strcmp(p, q) == 0, "keys are not equal");
#endif
}
END_TEST

START_TEST(test_hash_rpmqal)
{
    int nn = 100000;
    tn_hash *h;

    load_KEYS(nn);

    h = n_hash_new(nn, NULL);
    expect_notnull(h);
    n_hash_ctl(h, TN_HASH_NOCPKEY);

    //n_hash_ctl(h, TN_HASH_REHASH);
    nn += 4;
    clock_t t0 = clock();
    for (int i = 0; i < n_array_size(KEYS); i++) {
        struct str *st = n_array_nth(KEYS, i);
        n_hash_insert(h, st->s, st->s);
        //fail_unless(n_hash_exists(h, st->s), "%s: no such key", st->s);
    }
    printf_clock(t0, "insert");

    expect_int(n_hash_size(h), n_array_size(KEYS));

    t0 = clock();
    for (int i = 0; i < n_array_size(KEYS); i++) {
        struct str *st = n_array_nth(KEYS, i);
        fail_unless(n_hash_exists(h, st->s), "%s: no such key", st->s);

        char *v = n_hash_get(h, st->s);
        fail_unless (strcmp(v, st->s) == 0, "expected %s, got %s\n", st->s, v);
    }
    printf_clock(t0, "get+exists");

    t0 = clock();
    for (int i = nn; i < 2*nn; i++) {
        char key[40];
        sprintf(key, "key%20d", i);
        fail_if(n_hash_exists(h, key), "%s: should not be there", key);

        char *v = n_hash_get(h, key);
        fail_unless (v == NULL, "expected NULL\n");
    }
    printf_clock(t0, "non existing");
    n_hash_stats(h);
#ifdef TN_HASH_WITH_STORE
    const char *path = NTEST_TMPPATH("rpmqal.bin");
    t0 = clock();
    tn_stream *st = n_stream_open(path, "w", TN_STREAM_UNKNOWN);
    n_hash_store(h, st, store_str, NULL);
    n_stream_close(st);
    printf_clock(t0, "store");

    t0 = clock();
    st = n_stream_open(path, "r", TN_STREAM_UNKNOWN);
    n_hash_restore(st, restore_str, NULL, free);
    n_stream_close(st);
    printf_clock(t0, "restore");
#endif
    for (int i = 0; i < n_array_size(KEYS); i++) {
        struct str *st = n_array_nth(KEYS, i);
        char *v = n_hash_remove(h, st->s);
        fail_unless (strcmp(v, st->s) == 0, "expected %s, got %s\n", st->s, v);
    }

    expect_int(n_hash_size(h), 0);
    n_hash_free(h);
}
END_TEST

NTEST_RUNNER("hash", test_hash_base, test_hash_store, test_hash_rpmqal)
