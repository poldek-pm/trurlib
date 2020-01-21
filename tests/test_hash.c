/*
   $Id$
*/

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "n_check.h"
#include "nhash.h"

void hash_map_func(const char *key, void *data)
{
    char *k = (char *) key;
    char *d = (char *) data;
    printf("'%s' = '%s'\n", k, d);
}

START_TEST(test_hash_base)
{
    const char *VALUE = "ma kota i psa";
    int i, nn = 100000;
    tn_hash *h;

    h = n_hash_new(nn, NULL);
    expect_notnull(h);

    //n_hash_ctl(h, TN_HASH_REHASH);

    for (i = 0; i < nn; i++) {
        char key[40];
        sprintf(key, "key%d", i);
        /*if (i % 100000 == 0)
          printf("%s\n", key);*/

        n_hash_insert(h, key, VALUE);
        fail_unless(n_hash_exists(h, key), "%s: no such key", key);
    }
    expect_int(n_hash_size(h), nn);

    for (i = 0; i < nn; i++) {
        char key[40];
        sprintf(key, "key%d", i);
        fail_unless(n_hash_exists(h, key), "%s: no such key", key);

        char *v = n_hash_get(h, key);
        fail_unless (strcmp(v, VALUE) == 0, "expected %s, got %s\n", VALUE, v);
    }
    n_hash_stats(h);

    for (i = 0; i < nn; i++) {
        char key[40];
        sprintf(key, "key%d", i);
        /*if (i % 100000 == 0)
          printf("get %s\n", key);*/

        char *v = n_hash_remove(h, key);

        fail_unless(v, "%s: not removed?", key);
        fail_unless (strcmp(v, VALUE) == 0, "expected %s, got %s\n", VALUE, v);
    }

    expect_int(n_hash_size(h), 0);
    n_hash_free(h);
}
END_TEST

void test_hash_rpmqal(void)
{
    tn_hash *h;
    char buf[1024];
    FILE *f;

    if ((h = n_hash_new(600000, NULL)) == NULL)
        exit(1);

    //n_hash_ctl(h, TN_HASH_REHASH);
    f = popen("rpm -qal --requires --provides | grep -v buildid  | sort -u", "r");
    while (fgets(buf, sizeof(buf), f)) {
        buf[sizeof(buf) - 1] = '\0';
        n_hash_insert(h, buf, "ma kota i psa");
        if (!n_hash_exists(h, buf)) {
            printf("ERROR %s not exists\n", buf);
            exit(1);
        }
    }
    pclose(f);

    //  sleep(5);
//    n_hash_map(h, hash_map_func);
    n_hash_stats(h);
    n_hash_free(h);
}

NTEST_RUNNER("hash", test_hash_base);
