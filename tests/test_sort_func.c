/*
   $Id$
*/

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <sys/time.h>
#include <unistd.h>
#include "narray.h"
#include "hash/murmur3.c"
#include "hash/farmhash.c"

tn_array *KEYS = NULL;

struct str {
    int len;
    unsigned char s[];
};

int load_KEYS(int limit) {
    char buf[1024];

    KEYS = n_array_new(5 * 4096, free, (tn_fn_cmp)strcmp);

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

        //struct str *st = malloc(sizeof(*st) + len);
        //st->len = len - 1;
        //memcpy((char *)st->s, buf, len);
        //printf("buf %s %s\n", buf, st->s);
        n_array_push(KEYS, strdup(buf));
    }
    //fclose(stream);

    printf("Loaded %d keys\n", n_array_size(KEYS));

    return n_array_size(KEYS);
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

typedef void  (*sort_fn) (void**, size_t, tn_fn_cmp);

#define NLOOPS 10
void do_test(const char *name, sort_fn sort, tn_array *keys)
{
    int n = n_array_size(keys);

    tn_array *arrays[NLOOPS];
    for (int i = 0; i < NLOOPS; i++) {
        arrays[i] = n_array_dup(keys, (tn_fn_dup)strdup);
    }

    void *t = timethis_begin();
    for (int i = 0; i < NLOOPS; i++) {
        tn_array *a = arrays[i];
        sort(a->data, a->items, (tn_fn_cmp)strcmp);
        //for (int ii = 0; ii < 10; ii++) {
        //    printf("%d %s\n", ii, n_array_nth(a, ii));
        //}
    }
    timethis_end(t, name);
}

#pragma GCC diagnostic ignored "-Wcast-function-type"
int main()
{
    load_KEYS(0);

    do_test("trurl.qsort", trurl_qsort_voidp_arr, KEYS);
    do_test("trurl.isort", trurl_isort_voidp_arr, KEYS);
}
