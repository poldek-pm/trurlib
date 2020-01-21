/*
   $Id$
*/

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define CDB_HASHSTART 5381
uint32_t djb_hash(const char *s, int *slen)
{
    register uint32_t v;
    const char *ss = s;

    v = 0;
    while (*s) {
        v += (v << 5);
        v ^= (unsigned)*s;
        s++;
    }

    if (slen)
        *slen = s - ss;

    return v;
}

static inline uint32_t murmur_32_scramble(uint32_t k) {
    k *= 0xcc9e2d51;
    k = (k << 15) | (k >> 17);
    k *= 0x1b873593;
    return k;
}

uint32_t murmur3_hash(const uint8_t* key, int len)
{
    uint32_t h = CDB_HASHSTART ^ len;
    uint32_t k;

    /* Read in groups of 4. */
    for (size_t i = len >> 2; i; i--) {
        // Here is a source of differing results across endiannesses.
        // A swap here has no effects on hash properties though.
        k = *((uint32_t*)key);
        key += sizeof(uint32_t);
        h ^= murmur_32_scramble(k);
        h = (h << 13) | (h >> 19);
        h = h * 5 + 0xe6546b64;
    }
    /* Read the rest. */
    k = 0;
    for (size_t i = len & 3; i; i--) {
        k <<= 8;
        k |= key[i - 1];
    }
    // A swap is *not* necessary here because the preceeding loop already
    // places the low bytes in the low places according to whatever endianess
    // we use. Swaps only apply when the memory is copied in a chunk.
    h ^= murmur_32_scramble(k);
    /* Finalize. */
    h ^= len;
    h ^= h >> 16;
    h *= 0x85ebca6b;
    h ^= h >> 13;
    h *= 0xc2b2ae35;
    h ^= h >> 16;
    return h;
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

void test_hash_speed(void)
{
    char *s = "ncurses-devel-6.1.20191123-1.i686";
    int len = strlen(s);
    int i, v = 0;
    for (i = 0; i < 1000000000; i++) {
        //uint32_t h =  djb_hash(s, &len);
        uint32_t h =  murmur3_hash(s, len);
        if (i % 10000000 == 0)
            printf("%d, %s  => %u\n", i, s, h);
        v += h;
    }
    printf("%d, v  = %d\n", i, v);
}



int main()
{
    test_hash_speed();
    return 0;
}
