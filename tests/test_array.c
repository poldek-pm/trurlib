#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "n_snprintf.h"
#include "n_check.h"
#include "narray.h"
#undef n_array_push
#include "ndie.h"

// fail_unless macro woes
#pragma GCC diagnostic ignored "-Wformat-extra-args"

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

tn_array *setupArray(int size) {
    tn_array *a = n_array_new(size, free, (t_fn_cmp) strcmp);
    while (size > 0) {
        char buf[128];
        n_snprintf(buf, sizeof(buf), size > 100 ? "%3d" : (size > 10 ? "%2d" : "%d"), size);
        size--;
        n_array_push(a, strdup(buf));
    }
    return a;
}

char *setupArrayStr(int size) {
    char buf[16 * PATH_MAX];
    int n = 0;
    while (size > 0) {
        char buf[128];
        n += n_snprintf(&buf[n], sizeof(buf)-n, size > 100 ? "%3d," : (size > 10 ? "%2d," : "%d,"), size);
        size--;
    }
    buf[n] = '\0';
    return n_strdup(buf);
}
/*
char *setupArrayStrRev(int size) {
    char buf[16 * PATH_MAX];
    int n = 0;
    int i = 1;
    while (i <= size) {
        char buf[128];
        n += n_snprintf(&buf[n], sizeof(buf)-n, size > 100 ? "%3d," : (size > 10 "%2d," : "%d,"), i++);
    }
    buf[n] = '\0';
    return n_strdup(buf);
    }*/


START_TEST(test_array_concat)
{
    tn_array *a = setupArray(5);
    tn_array *b = setupArray(5);

    n_array_concat_ex(a, b, (tn_fn_dup)strdup);

    expect_str(array_str(a), "5,4,3,2,1,5,4,3,2,1");
    n_array_sort(a);
    expect_str(array_str(a), "1,1,2,2,3,3,4,4,5,5");

    n_array_uniq(a);
    expect_str(array_str(a), "1,2,3,4,5");

    n_array_free(a);
    n_array_free(b);
}
END_TEST


START_TEST(test_array_sort)
{
    tn_array *a = setupArray(5);

    expect_str(array_str(a), "5,4,3,2,1");
    n_array_sort(a);
    expect_str(array_str(a), "1,2,3,4,5");

    n_array_free(a);
}
END_TEST


START_TEST(test_array_sort_big)
{
    tn_array *a = setupArray(4096);
    n_array_sort(a);

    for (int i = 0; i < n_array_size(a)-1; i++) {
        char *e = n_array_nth(a, i);
        char *ee = n_array_nth(a, i+1);
        fail_unless(strcmp(e, ee) < 0, "not sorted");
    }

    n_array_free(a);
}
END_TEST


void isort(char **arr, size_t arr_size)
{
    register size_t i, j;

    for (i = 1; i < arr_size; i++) {
        register char *tmp = arr[i];

        j = i;
        while (j > 0 && strcmp(tmp, arr[j - 1]) < 0) {
            arr[j] = arr[j - 1];
            j--;
        }

        arr[j] = tmp;
    }
}

START_TEST(test_array_sort_stability)
{
    tn_array *a = setupArray(5);
    tn_array *b = setupArray(5);
    n_array_sort(b);


    expect_str(array_str(a), "5,4,3,2,1");
    n_array_concat_ex(a, b, (tn_fn_dup)strdup);
    n_array_push(a, strdup("1"));
    expect_str(array_str(a), "5,4,3,2,1,1,2,3,4,5,1");

    char *elem[n_array_size(a)];
    for (int i = 0; i < n_array_size(a); i++) {
        elem[i] = n_array_nth(a, i);
    }
    isort(elem, n_array_size(a));

    n_array_sort(a);
    expect_str(array_str(a), "1,1,1,2,2,3,3,4,4,5,5");

    for (int i = 0; i < n_array_size(a); i++) {
        char *e = elem[i];
        char *ee = n_array_nth(a, i);
        fail_unless(e == ee, "not stable sort");
    }

    n_array_free(a);
}
END_TEST

START_TEST(test_array_autosorting)
{
    tn_array *a = setupArray(5);
    expect_str(array_str(a), "5,4,3,2,1");

    expect_null(n_array_bsearch(a, "1"));
    n_array_ctl(a, TN_ARRAY_AUTOSORTED);
    expect_str(n_array_bsearch(a, "1"), "1");

    n_array_reverse(a);
    expect_str(array_str(a), "5,4,3,2,1");
    expect_str(n_array_bsearch(a, "2"), "2");

    n_array_free(a);
}
END_TEST


START_TEST(test_array_bsearch)
{
    tn_array *a = n_array_new(8, NULL, (t_fn_cmp) strcmp);
    n_array_push(a, "1");
    n_array_push(a, "2");

    expect_str(n_array_bsearch_ex(a, "1", (t_fn_cmp) strcmp), "1");
    expect_str(n_array_bsearch_ex(a, "2", (t_fn_cmp) strcmp), "2");
    expect_null(n_array_bsearch_ex(a, "3", (t_fn_cmp) strcmp));

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
    expect_null(n_array_push_inl(arr3, s1));
    expect_int(ndies, 2);

    n_array_free(arr1);
    n_array_free(arr2);
    n_array_free(arr3);
}
END_TEST

/* start_index + items stays within the block: without ASan a write past it
   can pass every other check */
#define expect_in_block(a) ck_assert_uint_le((a)->start_index + (a)->items, (a)->allocated)

/* the slots n_array_shift() leaves at the front are used again, the items
   stay inside the block */
static void check_push_after_shift(tn_array *(*push)(tn_array *, void *))
{
    tn_array *a = n_array_new(4, NULL, (t_fn_cmp) strcmp);

    push(a, "a");
    push(a, "b");
    push(a, "c");
    push(a, "d");
    n_array_shift(a);

    push(a, "e");
    expect_in_block(a);
    push(a, "f");
    expect_in_block(a);
    expect_str(array_str(a), "b,c,d,e,f");

    n_array_free(a);
}

START_TEST(test_array_push_after_shift)
{
    check_push_after_shift(n_array_push);     /* libtrurl's */
    check_push_after_shift(n_array_push_inl); /* narray.h's */
}
END_TEST

/* a,b,c,d in a block of the given size */
static tn_array *abcd_array(int size)
{
    tn_array *a = n_array_new(size, NULL, (t_fn_cmp) strcmp);

    n_array_push(a, "a");
    n_array_push(a, "b");
    n_array_push(a, "c");
    n_array_push(a, "d");
    return a;
}

START_TEST(test_array_concat_after_shift)
{
    tn_array *a, *src = n_array_new(2, NULL, (t_fn_cmp) strcmp);

    n_array_push(src, "e");
    n_array_push(src, "f");

    a = abcd_array(8);                  /* room at the end */
    n_array_shift(a);
    n_array_shift(a);
    n_array_concat(a, src);
    expect_int(n_array_size(a), 4);
    expect_str(array_str(a), "c,d,e,f");
    n_array_free(a);

    a = abcd_array(4);                  /* room once the items move */
    n_array_shift(a);
    n_array_shift(a);
    n_array_concat(a, src);
    expect_in_block(a);
    expect_int(n_array_size(a), 4);
    expect_str(array_str(a), "c,d,e,f");
    n_array_free(a);

    a = abcd_array(4);                  /* the block grows too */
    n_array_shift(a);
    n_array_concat(a, src);
    expect_in_block(a);
    expect_int(n_array_size(a), 5);
    expect_str(array_str(a), "b,c,d,e,f");
    n_array_free(a);

    n_array_free(src);
}
END_TEST

START_TEST(test_array_set_nth_past_end)
{
    tn_array *a = n_array_new(4, NULL, (t_fn_cmp) strcmp);

    n_array_set_nth(a, 0, "a");
    n_array_set_nth(a, 1, "b");
    n_array_set_nth(a, 2, "c");
    n_array_set_nth(a, 3, "d");
    n_array_set_nth(a, 4, "e");
    expect_in_block(a);
    expect_str(array_str(a), "a,b,c,d,e");

    n_array_shift(a);
    n_array_shift(a);
    n_array_set_nth(a, 3, "f");
    expect_str(array_str(a), "c,d,e,f");

    n_array_set_nth(a, 6, "g");         /* the items move to the front */
    expect_in_block(a);
    expect_int(n_array_size(a), 7);
    expect_str(n_array_nth(a, 3), "f");
    expect_null(n_array_nth(a, 5));
    expect_str(n_array_nth(a, 6), "g");

    n_array_free(a);
}
END_TEST

/* start_index + i can pass 16 bits: the bounds check must not truncate the
   sum to the width of start_index */
START_TEST(test_array_set_nth_past_16_bits)
{
    const int n = 70000;
    const int nshift = UINT16_MAX - 1; /* one short of n_array_shift() compacting */
    tn_array *a = n_array_new(4, NULL, NULL);

    for (int i = 0; i < n; i++)
        n_array_push(a, (void *)(intptr_t)i);
    for (int i = 0; i < nshift; i++)
        n_array_shift(a);

    expect_int(a->start_index, nshift);
    /* nshift + n is above UINT16_MAX and past the block: grows, compacts */
    a = n_array_set_nth(a, 70000, (void *)(intptr_t)1);
    expect_int(a->start_index, 0);
    expect_int(n_array_size(a), 70001);
    expect_int((intptr_t)n_array_nth(a, 0), nshift);
    expect_int((intptr_t)n_array_nth(a, n - nshift - 1), n - 1);
    expect_null(n_array_nth(a, n - nshift));
    expect_int((intptr_t)n_array_nth(a, 70000), 1);
    expect_in_block(a);

    n_array_free(a);
}
END_TEST

/* arrays made with no cmp_fn compare the pointers */
static char *s_x = "x", *s_y = "y", *s_c = "c", *s_d = "d", *s_e = "e", *s_f = "f";

/* c,d,e,f with two slots shifted out at the front */
static tn_array *shifted_array(void)
{
    tn_array *a = n_array_new(8, NULL, NULL);

    n_array_push(a, s_x);
    n_array_push(a, s_y);
    n_array_push(a, s_c);
    n_array_push(a, s_d);
    n_array_push(a, s_e);
    n_array_push(a, s_f);
    n_array_shift(a);
    n_array_shift(a);

    return a;
}

static char map_buf[16];
static void map_cat(void *s)
{
    strcat(map_buf, s);
}

START_TEST(test_array_ops_after_shift)
{
    tn_array *a, *b;

    a = shifted_array();
    map_buf[0] = '\0';
    n_array_map(a, map_cat);
    expect_str(map_buf, "cdef");

    b = n_array_new(4, NULL, NULL);
    n_array_push(b, s_c);
    n_array_push(b, s_d);
    n_array_push(b, s_e);
    n_array_push(b, s_f);
    expect_int(n_array_eq(a, b), 1);
    expect_int(n_array_eq(b, a), 1);
    n_array_free(b);

    n_array_reverse(a);
    expect_str(array_str(a), "f,e,d,c");
    n_array_free(a);

    a = shifted_array();
    n_array_remove_nth(a, 2);
    expect_str(array_str(a), "c,d,f");
    n_array_free(a);

    a = n_array_new(8, NULL, NULL);
    n_array_push(a, s_x);
    n_array_push(a, s_y);
    n_array_push(a, s_c);
    n_array_push(a, s_c);
    n_array_push(a, s_d);
    n_array_shift(a);
    n_array_shift(a);
    n_array_uniq(a);
    expect_str(array_str(a), "c,d");
    n_array_free(a);
}
END_TEST

/* start_index has 16 bits: shifting more items than that moves the rest back
   to the front of the block */
START_TEST(test_array_shift_many)
{
    const int n = 70000, nshift = 69990;
    tn_array *a = n_array_new(4, NULL, NULL);

    for (intptr_t i = 0; i < n; i++)
        n_array_push(a, (void *)i);

    for (intptr_t i = 0; i < nshift; i++) {
        void *p = n_array_shift(a);
        ck_assert_ptr_eq(p, (void *)i);
        expect_in_block(a);
    }

    expect_int(n_array_size(a), n - nshift);
    for (int i = 0; i < n - nshift; i++)
        ck_assert_ptr_eq(n_array_nth(a, i), (void *)(intptr_t)(nshift + i));

    n_array_push(a, (void *)(intptr_t)n);
    n_array_unshift(a, (void *)(intptr_t)(nshift - 1));
    expect_in_block(a);
    expect_int(n_array_size(a), n - nshift + 2);
    ck_assert_ptr_eq(n_array_nth(a, 0), (void *)(intptr_t)(nshift - 1));
    ck_assert_ptr_eq(n_array_nth(a, n_array_size(a) - 1), (void *)(intptr_t)n);

    n_array_free(a);
}
END_TEST

/* allocated is a uint32_t: a request past it dies instead of wrapping */
START_TEST(test_array_grow_limit)
{
    tn_array *a = n_array_new(4, NULL, NULL);

    a->allocated = UINT32_MAX / 2 + 1; /* fake; n_array_grow() must not realloc */

    ndies = 0;
    n_die_set_hook(die_hook);
    expect_null(n_array_grow(a, (size_t)UINT32_MAX / 2 + 2));
    expect_int(ndies, 1);
    n_die_set_hook(NULL);

    a->allocated = 4;
    n_array_free(a);
}
END_TEST

static int nasserts = 0;
static void assert_hook(const char *expr, const char *file, int line)
{
    (void)expr; (void)file; (void)line;
    nasserts++;
}

/* an array of no items given as data would have nothing to double */
START_TEST(test_array_new_empty_data)
{
    tn_array *a;

    n_assert_set_hook(assert_hook);
    nasserts = 0;
    a = n_array_new_ex(0, NULL, NULL, n_malloc(sizeof(void *)));
    expect_int(nasserts, 1);
    n_assert_set_hook(NULL);
    n_array_free(a);

    a = n_array_new_ex(0, NULL, NULL, NULL); /* no data: 0 is a default size */
    n_array_push(a, s_c);
    expect_str(array_str(a), "c");
    n_array_free(a);
}
END_TEST

/* an array on an allocator is freed with n_array_free_na() */
START_TEST(test_array_na)
{
    tn_alloc *na = n_alloc_new(4, TN_ALLOC_OBSTACK);
    tn_array *a = n_array_new_na(na, 2, NULL);

    n_array_push(a, s_c);
    n_array_push(a, s_d);
    n_array_push(a, s_e);
    expect_str(array_str(a), "c,d,e");

    n_array_free_na(na, a);
    n_alloc_free(na);
}
END_TEST

static int na_freed;
static void na_free_item(void *item)
{
    (void)item;
    na_freed++;
}

/* n_ref() counts an extra reference: the first n_array_free_na() only drops it */
START_TEST(test_array_na_refcnt)
{
    tn_alloc *na = n_alloc_new(4, TN_ALLOC_OBSTACK);
    tn_array *a = n_array_new_na(na, 2, NULL);

    n_array_ctl_set_freefn(a, na_free_item);
    n_array_push(a, s_c);
    n_array_push(a, s_d);
    n_ref(a);

    na_freed = 0;
    n_array_free_na(na, a);     /* drops n_ref()'s reference only */
    expect_int(na_freed, 0);
    expect_int(n_array_size(a), 2);
    expect_str(array_str(a), "c,d");

    n_array_free_na(na, a);     /* the owner's reference frees it */
    expect_int(na_freed, 2);
    n_alloc_free(na);
}
END_TEST

/* a frozen array is freed as n_array_free() frees one */
START_TEST(test_array_na_frozen)
{
    tn_alloc *na = n_alloc_new(4, TN_ALLOC_OBSTACK);
    tn_array *a = n_array_new_na(na, 2, NULL);

    n_array_ctl_set_freefn(a, na_free_item);
    n_array_push(a, s_c);
    n_array_push(a, s_d);
    n_array_freeze(a);

    na_freed = 0;
    n_array_free_na(na, a);
    expect_int(na_freed, 2);
    n_alloc_free(na);
}
END_TEST

static void *dup_same(void *ptr)
{
    return ptr;
}

/* a clone is malloc()ed, so it is freed with n_array_free() */
START_TEST(test_array_na_clone)
{
    tn_alloc *na = n_alloc_new(4, TN_ALLOC_OBSTACK);
    tn_array *a = n_array_new_na(na, 2, NULL);
    tn_array *c;

    n_array_push(a, s_c);
    n_array_push(a, s_d);

    c = n_array_clone_ex(a, dup_same);
    expect_str(array_str(c), "c,d");
    n_array_free(c);

    n_array_free_na(na, a);
    n_alloc_free(na);
}
END_TEST

/* a frozen source does not freeze the clone: it must stay fillable */
START_TEST(test_array_clone_frozen)
{
    tn_array *a = n_array_new(4, NULL, NULL);
    tn_array *c;

    n_array_push(a, s_c);
    n_array_freeze(a);

    c = n_array_clone_ex(a, dup_same);
    expect_str(array_str(c), "c");
    n_array_free(c);

    c = n_array_clone(a);       /* empty, for the caller to fill */
    n_array_push(c, s_d);
    expect_str(array_str(c), "d");
    n_array_free(c);

    n_array_free(a);
}
END_TEST

START_TEST(test_array_remove)
{
    tn_array *a = setupArray(5);

    n_array_reverse(a);
    expect_str(array_str(a), "1,2,3,4,5");

    n_array_remove(a, "3");
    expect_str(array_str(a), "1,2,4,5");

    n_array_remove_nth(a, 1);
    expect_str(array_str(a), "1,4,5");

    n_array_free(a);
}
END_TEST

static int nfreed = 0;
static void free_counted(void *p)
{
    nfreed++;
    free(p);
}

static int cmp_int(const void *a, const void *b)
{
    return *(const int *)a - *(const int *)b;
}

static int cmp_odd(const void *a, const void *b)
{
    (void)b;
    return *(const int *)a % 2 ? 0 : 1;
}

/* n ints from first, each of them repeated times, after a shift of one
   item (start_index > 0) */
static tn_array *int_array(int first, int n, int times)
{
    tn_array *a = n_array_new(4, free_counted, cmp_int);

    n_array_push(a, n_malloc(sizeof(int)));
    for (int i = 0; i < n; i++) {
        for (int k = 0; k < times; k++) {
            int *v = n_malloc(sizeof(*v));
            *v = first + i;
            n_array_push(a, v);
        }
    }
    free(n_array_shift(a));
    return a;
}

/* every item is checked once, the ones kept stay in order, each one removed
   is freed once, and the slots left behind are empty */
START_TEST(test_array_remove_many)
{
    const int n = 10000;
    tn_array *a = int_array(0, n, 1);

    nfreed = 0;
    n_array_remove_ex(a, NULL, cmp_odd);
    expect_int(n_array_size(a), n / 2);
    expect_int(nfreed, n / 2);
    for (int i = 0; i < n_array_size(a); i++)
        fail_unless(*(int *)n_array_nth(a, i) == 2 * i, "item %d is %d", i,
                    *(int *)n_array_nth(a, i));

    /* the array keeps working after it */
    int *v = n_malloc(sizeof(*v));
    *v = -1;
    n_array_push(a, v);
    expect_int(n_array_size(a), n / 2 + 1);
    expect_int(*(int *)n_array_nth(a, n / 2), -1);

    n_array_remove_ex(a, NULL, cmp_odd);    /* -1 is odd */
    expect_int(n_array_size(a), n / 2);

    int zero = 0;
    n_array_remove(a, &zero);
    expect_int(n_array_size(a), n / 2 - 1);
    expect_int(*(int *)n_array_nth(a, 0), 2);

    nfreed = 0;
    n_array_free(a);
    expect_int(nfreed, n / 2 - 1);
}
END_TEST

START_TEST(test_array_uniq_many)
{
    const int n = 10000;
    tn_array *a = int_array(0, n, 3);

    nfreed = 0;
    n_array_uniq(a);
    expect_int(n_array_size(a), n);
    expect_int(nfreed, 2 * n);
    for (int i = 0; i < n_array_size(a); i++)
        fail_unless(*(int *)n_array_nth(a, i) == i, "item %d is %d", i,
                    *(int *)n_array_nth(a, i));

    n_array_uniq(a);                        /* nothing to drop */
    expect_int(n_array_size(a), n);

    nfreed = 0;
    n_array_free(a);
    expect_int(nfreed, n);

    a = int_array(7, 1, 5);                 /* all the same */
    n_array_uniq(a);
    expect_int(n_array_size(a), 1);
    expect_int(*(int *)n_array_nth(a, 0), 7);
    n_array_free(a);
}
END_TEST

NTEST_RUNNER("array",
             test_array_basic,
             test_array_concat,
             test_array_sort,
             test_array_sort_stability,
             test_array_sort_big,
             test_array_autosorting,
             test_array_bsearch,
             test_array_growth,
             test_array_push_after_shift,
             test_array_concat_after_shift,
             test_array_set_nth_past_end,
             test_array_set_nth_past_16_bits,
             test_array_ops_after_shift,
             test_array_shift_many,
             test_array_grow_limit,
             test_array_new_empty_data,
             test_array_na,
             test_array_na_refcnt,
             test_array_na_frozen,
             test_array_na_clone,
             test_array_clone_frozen,
             test_array_remove,
             test_array_remove_many,
             test_array_uniq_many);
