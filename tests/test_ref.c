/*
  n_ref() counts more references than 16 bits hold: an object taken
  NREFS times keeps its contents through as many frees, and the next
  free releases it. poldek takes the allocator of a source and a few of
  its hashes and arrays once for each package.
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "nmalloc.h"
#include "narray.h"
#include "nbuf.h"
#include "nhash.h"
#include "nlist.h"
#include "noash.h"
#include "ntuple.h"
#include "n_obj_ref.h"
#include "n_check.h"

#define NREFS 70000

static void take_refs(void *o)
{
    int i;

    for (i = 0; i < NREFS; i++)
        fail_unless(n_ref(o) == o, "n_ref() returned another pointer");
}

#define FREE_REFS(free_call)                    \
    do {                                        \
        int i_;                                 \
        for (i_ = 0; i_ < NREFS; i_++)          \
            free_call;                          \
    } while (0)

START_TEST(test_ref_array)
{
    tn_array *arr = n_array_new(2, NULL, NULL);

    n_array_push(arr, "a");
    take_refs(arr);
    FREE_REFS(n_array_free(arr));
    expect_int(n_array_size(arr), 1);
    expect_str(n_array_nth(arr, 0), "a");
    n_array_free(arr);
}
END_TEST

START_TEST(test_ref_alloc)
{
    tn_alloc *na = n_alloc_new(4, TN_ALLOC_OBSTACK);
    char *s = na->na_malloc(na, 4);

    strcpy(s, "abc");
    take_refs(na);
    FREE_REFS(n_alloc_free(na));
    expect_str(s, "abc");
    s = na->na_malloc(na, 4);
    expect_notnull(s);
    n_alloc_free(na);
}
END_TEST

START_TEST(test_ref_buf)
{
    tn_buf *buf = n_buf_new(16);

    n_buf_puts(buf, "abc");
    take_refs(buf);
    FREE_REFS(n_buf_free(buf));
    expect_int(n_buf_size(buf), 3);
    n_buf_free(buf);
}
END_TEST

START_TEST(test_ref_tuple)
{
    void *data[] = { "a" };
    tn_tuple *tu = n_tuple_new(NULL, 1, data);

    take_refs(tu);
    FREE_REFS(n_tuple_free(NULL, tu));
    expect_int(n_tuple_size(tu), 1);
    expect_str(n_tuple_nth(tu, 0), "a");
    n_tuple_free(NULL, tu);
}
END_TEST

START_TEST(test_ref_hash)
{
    tn_hash *ht = n_hash_new(8, NULL);

    n_hash_insert(ht, "a", "x");
    take_refs(ht);
    FREE_REFS(n_hash_free(ht));
    expect_str(n_hash_get(ht, "a"), "x");
    n_hash_free(ht);
}
END_TEST

START_TEST(test_ref_oash)
{
    tn_oash *ht = n_oash_new(8, NULL);

    n_oash_insert(ht, "a", "x");
    take_refs(ht);
    FREE_REFS(n_oash_free(ht));
    expect_str(n_oash_get(ht, "a"), "x");
    n_oash_free(ht);
}
END_TEST

START_TEST(test_ref_list)
{
    tn_list *l = n_list_new(0, NULL, NULL);

    n_list_push(l, "a");
    take_refs(l);
    FREE_REFS(n_list_free(l));
    expect_int(n_list_size(l), 1);
    n_list_free(l);
}
END_TEST

NTEST_RUNNER("ref", test_ref_array, test_ref_alloc, test_ref_buf,
             test_ref_tuple, test_ref_hash, test_ref_oash, test_ref_list);
