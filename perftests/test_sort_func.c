
#define _POSIX_C_SOURCE 2
#include <alloca.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <sys/time.h>
#include <unistd.h>
#include "narray.h"

struct str {
    int len;
    unsigned char s[];
};

tn_array *load_keys(int limit) {
    char buf[1024];

    tn_array *keys = n_array_new(5 * 4096, free, (tn_fn_cmp)strcmp);

    FILE *stream = popen("rpm -qaRl --provides | cut -f1 -d' ' | sort -ur", "r");

    while (fgets(buf, sizeof(buf), stream)) {
        if (limit > 0 && n_array_size(keys) >= limit) {
            break;
        }

        int len = strlen(buf);
        if (len < 3) {
            continue;
        }
        buf[len-1] = '\0'; // \n

        //struct str *st = malloc(sizeof(*st) + len);
        //st->len = len - 1;
        //memcpy((char *)st->s, buf, len);
        //printf("buf %s %s\n", buf, st->s);
        n_array_push(keys, strdup(buf));
    }
    pclose(stream);
    //printf("Loaded %d keys\n", n_array_size(keys));

    return keys;
}

void *timethis_begin(void)
{
    struct timeval *tv;

    tv = malloc(sizeof(*tv));
    gettimeofday(tv, NULL);
    return tv;
}

void timethis_end(void *tvp, const char *prefix)
{
    struct timeval tv, *tv0 = (struct timeval *)tvp;

    gettimeofday(&tv, NULL);

    tv.tv_sec -= tv0->tv_sec;
    tv.tv_usec -= tv0->tv_usec;
    if (tv.tv_usec < 0) {
        tv.tv_sec--;
        tv.tv_usec = 1000000 + tv.tv_usec;
    }

    printf("%-16s %ld.%06lds\n", prefix, tv.tv_sec, tv.tv_usec);
    free(tvp);
}

#define SWAP_void(a, b)               \
            { register void *tmp = a; \
              a = b;                  \
              b = tmp;                \
            }

void trurl_qsort_voidp_arr(void **arr, size_t arr_size, t_fn_cmp cmpf)
{
    register unsigned i, j, ln, rn;

    while (arr_size > 1) {

	SWAP_void(arr[0], arr[arr_size / 2]);

	for (i = 0, j = arr_size;;) {

	    do {
		j--;
	    } while (j && cmpf(arr[j], arr[0]) > 0);


	    do {
		i++;
	    } while (i < j && cmpf(arr[i], arr[0]) < 0);

	    if (i >= j)
		break;

	    SWAP_void(arr[i], arr[j]);
	}

	SWAP_void(arr[j], arr[0]);

	ln = j;
	rn = arr_size - ++j;

	if (ln < rn) {
	    trurl_qsort_voidp_arr(arr, ln, cmpf);
	    arr += j;
	    arr_size = rn;

	} else {
	    trurl_qsort_voidp_arr(&arr[j], rn, cmpf);
	    arr_size = ln;
	}
    }
}


void trurl_isort_voidp_arr(void **arr, size_t arr_size, t_fn_cmp cmpf)
{
    register size_t i, j;

#if ENABLE_TRACE
    int n = 0;
    if (arr_size > 1000)
        DBGF("%d\n", arr_size);
#endif

    for (i = 1; i < arr_size; i++) {
        register void *tmp = arr[i];

        j = i;
#if ENABLE_TRACE
        if (arr_size > 1000 && i % 100 == 0)
            DBGF("(%d) iter = %d, n = %d\n", arr_size, i, n);
#endif

        while (j > 0 && cmpf(tmp, arr[j - 1]) < 0) {
            arr[j] = arr[j - 1];
            j--;
#if ENABLE_TRACE
            n++;
#endif
        }

        arr[j] = tmp;
    }
}

#define SORT_NAME void
#define SORT_TYPE void*
//#define SORT_CMP(x, y) strcmp(x, y)
#include "vendor/sort.h"

void htimsort_arr(void **arr, size_t arr_size, t_fn_cmp cmpf) {
    void_tim_sort(arr, arr_size, cmpf);
}

void hqsort_arr(void **arr, size_t arr_size, t_fn_cmp cmpf) {
    void_quick_sort(arr, arr_size, cmpf);
}

void hmergesort_arr(void **arr, size_t arr_size, t_fn_cmp cmpf) {
    void_merge_sort_in_place(arr, arr_size, cmpf);
}

typedef void  (*sort_fn) (void**, size_t, tn_fn_cmp);

void do_test(const char *name, sort_fn sort, tn_array *keys)
{
    n_array_size(keys);
    tn_array *arrays[1024] = {};
    int nloops = n_array_size(keys) <= 4096 ? 500 : 12;

    for (int i = 0; i < nloops; i++) {
        arrays[i] = n_array_dup(keys, (tn_fn_dup)strdup);
    }

    void *t = timethis_begin();
    for (int i = 0; i < nloops; i++) {
        tn_array *a = arrays[i];
        sort(a->data, a->items, (tn_fn_cmp)strcmp);
        //for (int ii = 0; ii < 10; ii++) {
        //    printf("%d %s\n", ii, n_array_nth(a, ii));
        //}
    }
    timethis_end(t, name);

    tn_array *a = arrays[0];
    n_assert(n_array_size(a) == n_array_size(keys));
    for (int i = 0; i < n_array_size(a)-1; i++) {
        int lt = strcmp(n_array_nth(a, i), n_array_nth(a, i+1)) < 0;
        if (!lt) {
            printf("%s bug [%d]%s > [%d]%s\n", name, i, (char *)n_array_nth(a, i), i+1, (char *)n_array_nth(a, i+1));
        }
    }

    for (int i = 0; i < nloops; i++) {
        n_array_free(arrays[i]);
    }
}

#pragma GCC diagnostic ignored "-Wcast-function-type"
int main()
{

    for (int i = (1<<8); i < (1<<20); i = i<<3) {
        tn_array *keys = load_keys(i);

        printf("N=%d, reversed\n", n_array_size(keys));
        do_test(" nqsort", trurl_qsort_voidp_arr, keys);
        do_test(" htimsort", htimsort_arr, keys);
        do_test(" hqsort", hqsort_arr, keys);
        do_test(" hmerge", hmergesort_arr, keys);
        if (n_array_size(keys) < 128)
            do_test(" isort", trurl_isort_voidp_arr, keys);


        printf("N=%d, sorted\n", n_array_size(keys));
        n_array_reverse(keys);
        do_test(" nqsort", trurl_qsort_voidp_arr, keys);
        do_test(" htimsort", htimsort_arr, keys);
        do_test(" hqsort", hqsort_arr, keys);
        do_test(" hmerge", hmergesort_arr, keys);
        if (n_array_size(keys) < 128)
            do_test(" isort", trurl_isort_voidp_arr, keys);


        for (int i = 0; i < n_array_size(keys); i++) {
            int x = rand() % n_array_size(keys);
            void *tmp = keys->data[x];
            keys->data[x] = keys->data[i];
            keys->data[i] = tmp;
        }

        printf("N=%d, randomized\n", n_array_size(keys));
        n_array_reverse(keys);
        do_test(" nqsort", trurl_qsort_voidp_arr, keys);
        do_test(" htimsort", htimsort_arr, keys);
        do_test(" hqsort", hqsort_arr, keys);
        do_test(" hmerge", hmergesort_arr, keys);
        if (n_array_size(keys) < 128)
            do_test(" isort", trurl_isort_voidp_arr, keys);

        n_array_free(keys);
    }
}
