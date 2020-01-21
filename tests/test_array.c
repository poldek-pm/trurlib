#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "n_snprintf.h"
#include "n_check.h"
#include "narray.h"
#undef n_array_push
#include "ndie.h"

static const char *array_str(const tn_array *a)
{
    static char buf[4096];
    int n = 0;
    for (int i = 0; i < n_array_size(a); i++) {
        n += n_snprintf(&buf[n], sizeof(buf) - n, "%s,", (char *)n_array_nth(a, i));
    }
    if (n > 0)
        buf[n - 1] = '\0';      /* trim last comma */

    return buf;
}

static int ndies = 0;
void die_hook(const char *msg) {
    (void)msg;
    ndies++;
}

tn_array *setupArray() {
    tn_array *a = n_array_new(8, free, (t_fn_cmp) strcmp);
    n_array_push(a, strdup("5"));
    n_array_push(a, strdup("4"));
    n_array_push(a, strdup("3"));
    n_array_push(a, strdup("2"));
    n_array_push(a, strdup("1"));
    return a;
}

START_TEST(test_array_sort)
{
    tn_array *a = setupArray();

    expect_str(array_str(a), "5,4,3,2,1");
    n_array_sort(a);
    expect_str(array_str(a), "1,2,3,4,5");

    n_array_free(a);
}
END_TEST


START_TEST(test_array_basic)
{
    tn_array *a;
    char *s;

    a = n_array_new(8, free, (t_fn_cmp) strcmp);

    n_array_push(a, n_strdup("Ala"));
    expect_int(n_array_size(a), 1);

    n_array_push(a, n_strdup("ma"));
    expect_int(n_array_size(a), 2);

    n_array_push(a, n_strdup("kota"));
    expect_int(n_array_size(a), 3);

    s = n_array_pop(a);
    expect_str(s, "kota");
    expect_int(n_array_size(a), 2);
    free(s);

    s = n_array_shift(a);
    expect_str(s, "Ala");
    free(s);

    n_array_unshift(a, n_strdup("Ola"));
    n_array_push(a, n_strdup("psa"));
    expect_int(n_array_size(a), 3);
    expect_str(array_str(a), "Ola,ma,psa");

    n_array_set_nth(a, 2, n_strdup("koty"));
    expect_str(array_str(a), "Ola,ma,koty");
    expect_int(n_array_size(a), 3);

    while (n_array_size(a)) {
        s = n_array_shift(a);

        if (s != NULL) {	/* arr contains some NULLs */
            free(s);
        }
    }
    expect_int(n_array_size(a), 0);

    ndies = 0;
    n_die_set_hook(die_hook);

    s = n_array_shift(a);
    expect_null(s);

    s = n_array_pop(a);
    expect_null(s);

    expect_int(ndies, 2);

    n_array_free(a);
}
END_TEST

START_TEST(test_array_growth)
{
    tn_array *arr1, *arr2, *arr3;
    char *s1 = "ala ";
    char *s2 = "ma ";
    char *s3 = "kota ";
    int i;

    arr1 = n_array_new(8, NULL, (t_fn_cmp) strcmp);
    arr2 = n_array_new(8, NULL, (t_fn_cmp) strcmp);
    arr3 = n_array_new(8, NULL, (t_fn_cmp) strcmp);
    n_array_ctl(arr3, TN_ARRAY_CONSTSIZE);

    for (i = 0; i < 8; i++) {
	n_array_push(arr1, s1);
	n_array_push(arr1, s2);
	n_array_push(arr1, s3);

	n_array_push(arr2, s1);
	n_array_push(arr2, s2);
	n_array_push(arr2, s3);

	n_array_push(arr3, s1);
    }
    expect_int(n_array_size(arr1), 24);
    expect_int(n_array_size(arr2), 24);
    expect_int(n_array_size(arr3), 8);
    //n_array_dump_stats(arr1, "arr1");
    //n_array_dump_stats(arr2, "arr2");
    //n_array_dump_stats(arr3, "arr3");

    //printf("Die test...\n");
    ndies = 0;
    n_die_set_hook(die_hook);
    expect_null(n_array_push(arr3, s1)); /* const array */
    expect_int(ndies, 1);

    n_array_free(arr1);
    n_array_free(arr2);
    n_array_free(arr3);
}
END_TEST

START_TEST(test_array_remove)
{
    tn_array *a = setupArray();

    n_array_reverse(a);
    expect_str(array_str(a), "1,2,3,4,5");

    n_array_remove(a, "3");
    expect_str(array_str(a), "1,2,4,5");

    n_array_remove_nth(a, 1);
    expect_str(array_str(a), "1,4,5");

    n_array_free(a);
}
END_TEST

NTEST_RUNNER("array",
             test_array_basic,
             test_array_sort,
             test_array_growth,
             test_array_remove);
