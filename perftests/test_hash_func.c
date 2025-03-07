/*
   $Id$
*/
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

#include "v/murmur3.c"
#include "v/farmhash.c"
#include <xxhash.h>

tn_array *KEYS = NULL;

struct str {
    int len;
    unsigned char s[];
};

int load_KEYS(int limit) {
    char buf[1024];

    KEYS = n_array_new(5 * 4096, free, NULL);

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


#define CDB_HASHSTART 5381
uint32_t djb_hash(const char *s, int len)
{
    register uint32_t v;
    (void)len;
    v = 0;
    while (*s) {
        v += (v << 5);
        v ^= (unsigned)*s;
        s++;
    }
    return v;
}

int sdbm_hash(const char *s, int *slen)
{
    register unsigned int v = 0;
    const char *ss = s;

    while (*s) {
        v = *s + (v << 6) + (v << 16) - v;
        s++;
    }

    if (slen)
        *slen = s - ss;

    return v;
}

void *timethis_begin(void)
{
    struct timeval *tv;

    tv = malloc(sizeof(*tv));
    gettimeofday(tv, NULL);
    return tv;
}

void timethis_end(void *tvp, const char *prefix, double cr)
{
    struct timeval tv, *tv0 = (struct timeval *)tvp;

    gettimeofday(&tv, NULL);

    tv.tv_sec -= tv0->tv_sec;
    tv.tv_usec -= tv0->tv_usec;
    if (tv.tv_usec < 0) {
        tv.tv_sec--;
        tv.tv_usec = 1000000 + tv.tv_usec;
    }

    printf("%-16s %ld.%06lds %.2lfcr\n", prefix, tv.tv_sec, tv.tv_usec, 100.0 * cr);
    free(tvp);
}

typedef uint32_t (*hash_fn)(unsigned char *, int);

void do_test(const char *name, hash_fn fn, tn_array *keys)
{
    int n = n_array_size(keys);
    int ncolisions = 0;
    uint32_t size = n, rsize = 16;

    while (rsize < size) {
        rsize <<= 1;
    }
    size = rsize;

    int *collisions = alloca(size * sizeof(int));
    memset(collisions, 0, size * sizeof(int));

    void *t = timethis_begin();
    for (int i = 0; i < n_array_size(KEYS); i++) {
        struct str *st = n_array_nth(KEYS, i);
        uint32_t h = fn(st->s, st->len);
        uint32_t index = h & (size - 1);
        //printf("%d %s %u %u %s\n", i, st->s, h, index, collisions[index] > 0 ? "CO" : "");
        if (collisions[index] > 0) {
            ncolisions++;
        }
        collisions[index]++;
    }
    timethis_end(t, name, ncolisions/(size * 1.0));
}

uint32_t xxhash(unsigned char *s, int len) {
    return XXH64(s, len, 0);
}

#include "v/metrohash.h"
#include "v/metrohash64.c"
uint32_t metrohash(unsigned char *s, int len) {
    uint64_t h = 0;
    metrohash64_1(s, len, 0, (uint8_t*)&h);
    return h;
}


#pragma GCC diagnostic ignored "-Wcast-function-type"
int main()
{
    load_KEYS(0);

    do_test("murmur3", (hash_fn)&murmur3_hash, KEYS);
    do_test("farmhash32", (hash_fn)&farmhash32, KEYS);
    do_test("farmhash64", (hash_fn)&farmhash64, KEYS);
    do_test("metrohash64", (hash_fn)&metrohash, KEYS);
    do_test("xxhash", (hash_fn)&xxhash, KEYS);
    do_test("djb", (hash_fn)&djb_hash, KEYS);
}
