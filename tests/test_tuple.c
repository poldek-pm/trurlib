/*
  A tuple holds more elements than 16 bits count: poldek keeps one for
  each directory of a package in its file list.
*/
#include <stdio.h>
#include <stdlib.h>

#include "nmalloc.h"
#include "ntuple.h"
#include "n_check.h"

#define NELEM 70000

START_TEST(test_tuple_large)
{
    tn_tuple *tu = n_tuple_new(NULL, NELEM, NULL);
    int i;

    expect_int(n_tuple_size(tu), NELEM);
    expect_null(n_tuple_nth(tu, NELEM - 1));
    for (i = 0; i < NELEM; i++)
        n_tuple_set_nth(tu, i, (void *)(long)(i + 1));
    expect_int((long)n_tuple_nth(tu, 0), 1);
    expect_int((long)n_tuple_nth(tu, NELEM - 1), NELEM);
    n_tuple_free(NULL, tu);
}
END_TEST

START_TEST(test_tuple_large_copy)
{
    void **data = n_malloc(NELEM * sizeof(*data));
    tn_tuple *tu;
    int i;

    for (i = 0; i < NELEM; i++)
        data[i] = (void *)(long)(i + 1);
    tu = n_tuple_new(NULL, NELEM, data);
    n_free(data);
    expect_int(n_tuple_size(tu), NELEM);
    expect_int((long)n_tuple_nth(tu, NELEM - 1), NELEM);
    n_tuple_free(NULL, tu);
}
END_TEST

NTEST_RUNNER("tuple", test_tuple_large, test_tuple_large_copy);
