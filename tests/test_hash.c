/*
   $Id$
*/


#include <errno.h>
#define __USE_POSIX2 1
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>

#include "n_check.h"
#include "n_snprintf.h"
#include "nhash.h"
#include "noash.h"
#include "nstream.h"
#include "n_hash_int.h"

// fail_unless macro woes
#pragma GCC diagnostic ignored "-Wformat-extra-args"
static char *array_str(const tn_array *a);

void hash_map_func(const char *key, void *data)
{
    char *k = (char *) key;
    char *d = (char *) data;
    printf("'%s' = '%s'\n", k, d);
}

int n_hash_dump(const tn_hash *ht);
int n_oash_dump(const tn_oash *ht);


START_TEST(test_hash_base)
{
    int nn = 512;
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

    /* insert */
    for (int i = 0; i < nn; i++) {
        const char *key = keys[i];
        n_hash_insert(h, key, strdup(key));
        //n_hash_dump(h);
        fail_unless(n_hash_exists(h, key), "%s: no such key", key);
    }
    expect_int(n_hash_size(h), nn);
    //n_hash_dump(h);

    /* test iterator */
    tn_array *given_keys = n_array_new(16, NULL, (tn_fn_cmp)strcmp);
    for (int i = 0; i < nn; i++) {
        char *key = keys[i];
        n_array_push(given_keys, key);
    }

    tn_array *actual_keys = n_array_new(16, NULL, (tn_fn_cmp)strcmp);
    tn_hash_it it;
    n_hash_it_init(&it, h);
    const char *k = NULL;

    while (n_hash_it_get(&it, &k)) {
        n_array_push(actual_keys, (char *)k);
    }

    n_array_sort(given_keys);
    n_array_sort(actual_keys);
    fail_unless(n_array_size(actual_keys) == nn, "iterator fail %d != %d", n_array_size(actual_keys), nn);

    char *p, *q;
    p = array_str(given_keys);
    q = array_str(actual_keys);
    fail_unless(strcmp(p, q) == 0, "iterator fail");

    /* test get/ exists */
    for (int i = 0; i < nn + 10; i++) {
        const char *key = keys[i];
        const char *v = n_hash_get(h, key);
        if (i < nn)
            fail_unless(v && strcmp(v, key) == 0, "%s: no such key", key);
        else
            fail_if(n_hash_exists(h, key), "%s: should not be found", key);
    }

    //n_hash_dump(h);
    /* test remove */
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
            fail_if(n_hash_exists(h, s), "%s: should not be found", key);
        }

        for (int j = i+1; j < nn; j++) {
            const char *s = keys[j];
            fail_unless(n_hash_exists(h, s), "%s: should be found", key);
        }
    }
    expect_int(n_hash_size(h), 0);
}
END_TEST

START_TEST(test_ohash_base)
{
    int nn = 32;
    char *keys[4*nn];
    tn_oash *h;

    for (int i = 0; i < 4*nn; i++) {
        char key[64];
        snprintf(key, sizeof(key), "key%02d", i);
        keys[i] = strdup(key);
    }

    h = n_oash_new(nn * 2, NULL);
    n_oash_ctl(h, TN_HASH_NOCPKEY);

    if (nn < 20)
        n_oash_dump(h);

    for (int i = 0; i < nn; i++) {
        const char *key = keys[i];
        n_oash_insert(h, key, &key[1]);
        if (nn < 20)
            n_oash_dump(h);
        fail_unless(n_oash_exists(h, key), "%s: no such key", key);
    }
    expect_int(n_oash_size(h), nn);

    for (int i = 0; i < nn; i++) {
        const char *key = keys[i];
        void **data = n_oash_get_insert(h, key, strlen(key));

        if (nn < 20)
            n_oash_dump(h);

        fail_if(data == NULL, "get_insert should not return NULL");
        fail_unless(*data == &key[1], "%s's value should not be updated", key);
    }
    expect_int(n_oash_size(h), nn);

    /* test iterator */
    tn_array *given_keys = n_array_new(16, NULL, (tn_fn_cmp)strcmp);
    for (int i = 0; i < nn; i++) {
        char *key = keys[i];
        n_array_push(given_keys, key);
    }

    tn_array *actual_keys = n_array_new(16, NULL, (tn_fn_cmp)strcmp);
    tn_oash_it it;
    n_oash_it_init(&it, h);
    const char *k = NULL;

    while (n_oash_it_get(&it, &k)) {
        n_array_push(actual_keys, (char *)k);
    }

    n_array_sort(given_keys);
    n_array_sort(actual_keys);
    fail_unless(n_array_size(actual_keys) == nn, "iterator fail n");

    char *p, *q;
    p = array_str(given_keys);
    q = array_str(actual_keys);
    fail_unless(strcmp(p, q) == 0, "iterator fail");

    // replace
    for (int i = 0; i < nn; i++) {
        const char *key = keys[i];
        n_oash_replace(h, key, &key[2]);
        const char *data = n_oash_get(h, key);
        if (nn < 20)
            n_oash_dump(h);
        fail_unless(data == &key[2], "%s's should not be updated", key);

    }
    expect_int(n_oash_size(h), nn);

    if (nn < 20)
        n_oash_dump(h);

    for (int i = 0; i < nn + 10; i++) {
        const char *key = keys[i];
        const char *v = n_oash_get(h, key);
        if (i < nn)
            fail_unless(v && strcmp(v, &key[2]) == 0, "%s: no such key, got %s", key, v);
        else
            fail_if(n_oash_exists(h, key), "%s: should not be found", key);
    }

    for (int i = 0; i < nn; i++) {
        const char *key = keys[i];
        const char **v = (const char **)n_oash_get_insert(h, key, strlen(key));
        fail_unless(v && *v && strcmp(*v, &key[2]) == 0, "%s: no such key, got %s", key, v ? *v : "NULL");
    }


    //n_hash_dump(h);
    for (int i = 0; i < nn; i++) {
        const char *key = keys[i];
        const char *v = n_oash_remove(h, key);
        if (i < nn) {
            fail_if(n_oash_exists(h, key), "%s: should not be found", key);
            fail_unless(strcmp(v, &key[2]) == 0, "%s: should eq %s", key, v);
            //n_assert(strcmp(v, key) == 0);
        }
        if (nn < 20)
            n_oash_dump(h);

        for (int j = 0; j < i+1; j++) {
            const char *s = keys[j];
            fail_if(n_oash_exists(h, s), "%s: should not be found", key);
        }

        for (int j = i+1; j < nn; j++) {
            const char *s = keys[j];
            fail_unless(n_oash_exists(h, s), "%s: should be found", key);
        }
    }
    expect_int(n_oash_size(h), 0);


    // rehash
    n_oash_ctl(h, TN_HASH_REHASH);
    for (int i = 0; i < 4*nn; i++) {
        const char *key = keys[i];
        n_oash_insert(h, key, key);
    }
    expect_int(n_oash_size(h), 4*nn);

    for (int i = 0; i < 4*nn; i++) {
        const char *key = keys[i];
        const char *v = n_oash_get(h, key);
        fail_unless(v == key, "%s: no such key", key);
    }

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

START_TEST(test_ohash_store)
{
    int nn = 32;
    char *keys[nn + 10];
    tn_oash *h;

    for (int i = 0; i < nn + 10; i++) {
        char key[64];
        snprintf(key, sizeof(key), "key%02d", i);
        keys[i] = strdup(key);
    }

    h = n_oash_new(2*nn, free);
    n_oash_ctl(h, TN_HASH_NOCPKEY);
    //n_oash_ctl(h, TN_HASH_REHASH);

    for (int i = 0; i < nn; i++) {
        const char *key = keys[i];
        n_oash_insert(h, strdup(key), strdup(keys[(i + i/2) % nn]));
        //n_oash_dump(h);
        fail_unless(n_oash_exists(h, key), "%s: no such key", key);
    }

    expect_int(n_oash_size(h), nn);

    //n_oash_dump(h);
    const char *path = NTEST_TMPPATH("hash.bin.gz");
    //printf("pp %s\n", path);
    //n_oash_dump(h);

    tn_stream *st = n_stream_open(path, "w", TN_STREAM_UNKNOWN);
    n_oash_store(h, st, store_str, NULL);
    n_stream_close(st);

    //printf("stored %s\n", path);

    st = n_stream_open(path, "r", TN_STREAM_UNKNOWN);
    tn_oash *rh = n_oash_restore(st, restore_str, NULL, free);
    n_stream_close(st);
    fail_if(rh == NULL, "n_oash_restore should succeed");
    //n_oash_dump(rh);
    fail_if(n_oash_size(rh) != n_oash_size(h), "n_oash_restore: sizes should be equal");

    char *p, *q;
    p = array_str(n_oash_values(h));
    q = array_str(n_oash_values(rh));

    fail_unless(strcmp(p, q) == 0, "values are not equal");

    p = array_str(n_oash_keys(h));
    q = array_str(n_oash_keys(rh));
    fail_unless(strcmp(p, q) == 0, "keys are not equal");
}
END_TEST

NTEST_RUNNER("hash",
             test_hash_base,
             test_ohash_base,
             test_ohash_store
           )
